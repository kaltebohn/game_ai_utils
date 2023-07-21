#include "softmax.hpp"

std::vector<double> softmax(std::vector<double> scores) {
  /* 各得点のexpを取る。ただし、オーバーフローを防ぐため最大得点で引いた上で計算する。 */
  const double max_score{*std::max_element(scores.begin(), scores.end())};
  std::transform(scores.begin(), scores.end(), scores.begin(),
      [max_score](const double score) { return exp(score - max_score); });

  /* 各分子を分母で割って、ソフトマックス値を計算。 */
  const double sum_score{std::accumulate(scores.begin(), scores.end(), 0.0)};
  std::transform(scores.begin(), scores.end(), scores.begin(),
      [sum_score](const double score) { return (score / sum_score); });

  return scores;
}
