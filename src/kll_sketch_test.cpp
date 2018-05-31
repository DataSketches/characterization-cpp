#include <kll_sketch.hpp>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <random>
#include <chrono>
#include <sstream>
#include <cstring>

/*
 * Computes the next larger integer point in the power series
 * <i>point = 2<sup>( i / ppo )</sup></i> given the current point in the series.
 * For illustration, this can be used in a loop as follows:
 *
 * <pre>{@code
 *     int maxP = 1024;
 *     int minP = 1;
 *     int ppo = 2;
 *
 *     for (int p = minP; p <= maxP; p = pwr2LawNext(ppo, p)) {
 *       System.out.print(p + " ");
 *     }
 *     //generates the following series:
 *     //1 2 3 4 6 8 11 16 23 32 45 64 91 128 181 256 362 512 724 1024
 * }</pre>
 *
 * param ppo Points-Per-Octave, or the number of points per integer powers of 2 in the series.
 * param curPoint the current point of the series. Must be &ge; 1.
 * returns the next point in the power series.
 */
size_t pwr_2_law_next(size_t ppo, size_t cur_point) {
  const size_t cur = (cur_point < 1) ? 1 : cur_point;
  size_t gi(round(log2(cur) * ppo)); //current generating index
  size_t next;
  do {
    next = (size_t) round(pow(2.0, (double) ++gi / ppo));
  } while ( next <= cur_point);
  return next;
}

/*
 * Counts the actual number of plotting points between lgStart and lgEnd assuming the given PPO.
 * This is not a simple linear function due to points that may be skipped in the low range.
 * param lgStart Log2 of the starting value
 * param lgEnd Log2 of the ending value
 * param ppo the number of logrithmically evenly spaced points per octave.
 * returns the actual number of plotting points between lgStart and lgEnd.
 */
size_t count_points(size_t lg_start, size_t lg_end, size_t ppo) {
  size_t p(1 << lg_start);
  const size_t end(1 << lg_end);
  size_t count(0);
  while (p <= end) {
    p = pwr_2_law_next(ppo, p);
    count++;
  }
  return count;
}

double accuracy_trial(float* values, size_t stream_length) {
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::shuffle(values, values + stream_length, std::default_random_engine(seed));

  sketches::kll_sketch sketch;
  for (size_t i = 0; i < stream_length; i++) sketch.update(values[i]);

  double max_rank_error = 0;
  for (size_t i = 0; i < stream_length; i++) {
    double true_rank = (double) i / stream_length;
    double est_rank = sketch.get_rank(i);
    max_rank_error = std::max(max_rank_error, abs(true_rank - est_rank));
  }

  return max_rank_error;
}

void accuracy(void) {
  const size_t lg_min(0);
  const size_t lg_max(23);
  const size_t ppo(16);
  const size_t num_trials(100);
  const size_t error_pct(99);

  double rank_errors[num_trials];

  size_t max_len(1 << lg_max);
  float* values = new float[max_len];

  const size_t num_steps = count_points(lg_min, lg_max, ppo);
  size_t stream_length(1 << lg_min);
  for (size_t i = 0; i < num_steps; i++) {
    for (size_t i = 0; i < stream_length; i++) values[i] = i;

    for (size_t t = 0; t < num_trials; t++) {
      const double maxRankErrorInTrial = accuracy_trial(values, stream_length);
      rank_errors[t] = maxRankErrorInTrial;
    }

    std::sort(&rank_errors[0], &rank_errors[num_trials]);
    const size_t error_pct_index = num_trials * error_pct / 100;
    const double rank_error = rank_errors[error_pct_index];

    std::cout << stream_length << "\t" << rank_error * 100 << std::endl;

    stream_length = pwr_2_law_next(ppo, stream_length);
  }
  delete [] values;
}

size_t getNumTrials(size_t x, size_t lgMinX, size_t lgMaxX, size_t lgMinTrials, size_t lgMaxTrials) {
  const double slope = (double) (lgMaxTrials - lgMinTrials) / ((int) lgMinX - (int) lgMaxX);
  const double lgTrials = (slope * log2(x)) + lgMaxTrials;
  return (size_t) pow(2, lgTrials);
}

void timing(void) {
  const size_t lg_min_stream_len(0);
  const size_t lg_max_stream_len(23);
  const size_t ppo(16);

  const size_t lg_max_trials(16);
  const size_t lg_min_trials(6);

  const size_t num_queries(20);

  std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
  std::uniform_real_distribution<float> distribution(0.0, 1.0);

  std::cout << "Stream\tTrials\tBuild\tUpdate\tQuant\tQuants\tRank\tCDF\tSer\tDeser\tItems\tSize" << std::endl;

  size_t max_len(1 << lg_max_stream_len);
  float* values = new float[max_len];

  float rank_query_values[num_queries];
  for (size_t i = 0; i < num_queries; i++) rank_query_values[i] = distribution(generator);
  std::sort(&rank_query_values[0], &rank_query_values[num_queries]);

  double quantile_query_values[num_queries];
  for (size_t i = 0; i < num_queries; i++) quantile_query_values[i] = distribution(generator);

  size_t stream_length(1 << lg_min_stream_len);
  while (stream_length <= (1 << lg_max_stream_len)) {

    std::chrono::nanoseconds build_time_ns(0);
    std::chrono::nanoseconds update_time_ns(0);
    std::chrono::nanoseconds get_quantile_time_ns(0);
    std::chrono::nanoseconds get_quantiles_time_ns(0);
    std::chrono::nanoseconds get_rank_time_ns(0);
    std::chrono::nanoseconds get_cdf_time_ns(0);
    std::chrono::nanoseconds serialize_time_ns(0);
    std::chrono::nanoseconds deserialize_time_ns(0);
    size_t num_retained(0);
    size_t size_bytes(0);

    const size_t num_trials = getNumTrials(stream_length, lg_min_stream_len, lg_max_stream_len, lg_min_trials, lg_max_trials);
    for (size_t i = 0; i < num_trials; i++) {
      for (size_t i = 0; i < stream_length; i++) values[i] = distribution(generator);

      auto start_build(std::chrono::high_resolution_clock::now());
      sketches::kll_sketch sketch;
      auto finish_build(std::chrono::high_resolution_clock::now());
      build_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_build - start_build);

      auto start_update(std::chrono::high_resolution_clock::now());
      for (size_t i = 0; i < stream_length; i++) sketch.update(values[i]);
      auto finish_update(std::chrono::high_resolution_clock::now());
      update_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_update - start_update);

      auto start_get_quantile(std::chrono::high_resolution_clock::now());
      for (size_t i = 0; i < num_queries; i++) sketch.get_quantile(quantile_query_values[i]);
      auto finish_get_quantile(std::chrono::high_resolution_clock::now());
      get_quantile_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_get_quantile - start_get_quantile);

      auto start_get_quantiles(std::chrono::high_resolution_clock::now());
      sketch.get_quantiles(quantile_query_values, num_queries);
      auto finish_get_quantiles(std::chrono::high_resolution_clock::now());
      get_quantiles_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_get_quantiles - start_get_quantiles);

      auto start_get_rank(std::chrono::high_resolution_clock::now());
      for (size_t i = 0; i < num_queries; i++) sketch.get_rank(rank_query_values[i]);
      auto finish_get_rank(std::chrono::high_resolution_clock::now());
      get_rank_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_get_rank - start_get_rank);

      auto start_get_cdf(std::chrono::high_resolution_clock::now());
      sketch.get_CDF(rank_query_values, num_queries);
      auto finish_get_cdf(std::chrono::high_resolution_clock::now());
      get_cdf_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_get_cdf - start_get_cdf);

      std::stringstream s(std::ios::in | std::ios::out | std::ios::binary);
      auto start_serialize(std::chrono::high_resolution_clock::now());
      sketch.serialize(s);
      auto finish_serialize(std::chrono::high_resolution_clock::now());
      serialize_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_serialize - start_serialize);

      auto start_deserialize(std::chrono::high_resolution_clock::now());
      auto sketch_ptr(sketches::kll_sketch::deserialize(s));
      auto finish_deserialize(std::chrono::high_resolution_clock::now());
      deserialize_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_deserialize - start_deserialize);

      num_retained += sketch.get_num_retained();
      size_bytes += s.tellp();
    }
    std::cout << stream_length << "\t"
        << num_trials << "\t"
        << (double) build_time_ns.count() / num_trials << "\t"
        << (double) update_time_ns.count() / num_trials / stream_length << "\t"
        << (double) get_quantile_time_ns.count() / num_trials / num_queries << "\t"
        << (double) get_quantiles_time_ns.count() / num_trials / num_queries << "\t"
        << (double) get_rank_time_ns.count() / num_trials / num_queries << "\t"
        << (double) get_cdf_time_ns.count() / num_trials / num_queries << "\t"
        << (double) serialize_time_ns.count() / num_trials << "\t"
        << (double) deserialize_time_ns.count() / num_trials << "\t"
        << num_retained / num_trials << "\t"
        << size_bytes / num_trials << std::endl;
    stream_length = pwr_2_law_next(ppo, stream_length);
  }
  delete [] values;
}

int main(int argc, char **argv) {
  if (argc == 2) {
    if (strcmp(argv[1], "accuracy") == 0) {
      accuracy();
    } else if (strcmp(argv[1], "timing") == 0) {
      timing();
    } else {
      std::cerr << "Unsupported command " << argv[1] << std::endl;
    }
  } else {
    std::cerr << "Only one parameter expected: accuracy or timing" << std::endl;
  }
  return 0;
}
