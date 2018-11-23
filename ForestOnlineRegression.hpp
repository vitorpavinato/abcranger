#pragma once

#include <iostream>
#include <vector>

#include "globals.h"
#include "ForestOnline.hpp"

namespace ranger {

class ForestOnlineRegression: public ForestOnline {
public:
  ForestOnlineRegression() = default;

  ForestOnlineRegression(const ForestOnlineRegression&) = delete;
  ForestOnlineRegression& operator=(const ForestOnlineRegression&) = delete;

  virtual ~ForestOnlineRegression() override = default;

private:
  void initInternal(std::string status_variable_name) override;
  void growInternal() override;
  void calculateAfterGrow(size_t tree_idx) override;
  void allocatePredictMemory() override;
  void predictInternal(size_t sample_idx) override;
  void computePredictionErrorInternal() override;
  void writeOutputInternal() override;
  void writeConfusionFile() override;
  void writePredictionFile() override;
  void saveToFileInternal(std::ofstream& outfile) override;
  void loadFromFileInternal(std::ifstream& infile) override;

  // OOb counts for regression
  std::vector<size_t> samples_oob_count;

private:
  double getTreePrediction(size_t tree_idx, size_t sample_idx) const;
  size_t getTreePredictionTerminalNodeID(size_t tree_idx, size_t sample_idx) const;
};

} // namespace ranger