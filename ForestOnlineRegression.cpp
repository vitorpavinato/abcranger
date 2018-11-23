#include <algorithm>
#include <stdexcept>
#include <string>

#include "utility.h"
#include "ForestOnlineRegression.hpp"
#include "TreeRegression.h"
#include "Data.h"

namespace ranger {

void ForestOnlineRegression::initInternal(std::string status_variable_name) {

  // If mtry not set, use floored square root of number of independent variables
  if (mtry == 0) {
    unsigned long temp = sqrt((double) (num_variables - 1));
    mtry = std::max((unsigned long) 1, temp);
  }

  // Set minimal node size
  if (min_node_size == 0) {
    min_node_size = DEFAULT_MIN_NODE_SIZE_REGRESSION;
  }

  // Sort data if memory saving mode
  if (!memory_saving_splitting) {
    data->sort();
  }
}

void ForestOnlineRegression::growInternal() {
  trees.reserve(num_trees);
  for (size_t i = 0; i < num_trees; ++i) {
    trees.push_back(make_unique<TreeRegression>());
  }

  predictions = std::vector<std::vector<std::vector<double>>>(1,
      std::vector<std::vector<double>>(1, std::vector<double>(num_samples, 0)));
  samples_oob_count.resize(num_samples, 0);

}

void ForestOnlineRegression::allocatePredictMemory() {
  size_t num_prediction_samples = data->getNumRows();
  if (predict_all || prediction_type == TERMINALNODES) {
    predictions = std::vector<std::vector<std::vector<double>>>(1,
        std::vector<std::vector<double>>(num_prediction_samples, std::vector<double>(num_trees)));
  } else {
    predictions = std::vector<std::vector<std::vector<double>>>(1,
        std::vector<std::vector<double>>(1, std::vector<double>(num_prediction_samples)));
  }
}

void ForestOnlineRegression::predictInternal(size_t sample_idx) {
  if (predict_all || prediction_type == TERMINALNODES) {
    // Get all tree predictions
    for (size_t tree_idx = 0; tree_idx < num_trees; ++tree_idx) {
      if (prediction_type == TERMINALNODES) {
        predictions[0][sample_idx][tree_idx] = getTreePredictionTerminalNodeID(tree_idx, sample_idx);
      } else {
        predictions[0][sample_idx][tree_idx] = getTreePrediction(tree_idx, sample_idx);
      }
    }
  } else {
    // Mean over trees
    double prediction_sum = 0;
    for (size_t tree_idx = 0; tree_idx < num_trees; ++tree_idx) {
      prediction_sum += getTreePrediction(tree_idx, sample_idx);
    }
    predictions[0][0][sample_idx] = prediction_sum / num_trees;
  }
}

void ForestOnlineRegression::calculateAfterGrow(size_t tree_idx) {
  // For each tree loop over OOB samples and count classes
    for (size_t sample_idx = 0; sample_idx < trees[tree_idx]->getNumSamplesOob(); ++sample_idx)
    {
      size_t sampleID = trees[tree_idx]->getOobSampleIDs()[sample_idx];
      double value = getTreePrediction(tree_idx, sample_idx);

      predictions[0][0][sampleID] += value;
      ++samples_oob_count[sampleID];
    }
}

void ForestOnlineRegression::computePredictionErrorInternal() {

// For each sample sum over trees where sample is OOB
  // for (size_t tree_idx = 0; tree_idx < num_trees; ++tree_idx) {
  //   for (size_t sample_idx = 0; sample_idx < trees[tree_idx]->getNumSamplesOob(); ++sample_idx) {
  //     size_t sampleID = trees[tree_idx]->getOobSampleIDs()[sample_idx];
  //     double value = getTreePrediction(tree_idx, sample_idx);

  //     predictions[0][0][sampleID] += value;
  //     ++samples_oob_count[sampleID];
  //   }
  // }

// MSE with predictions and true data
  size_t num_predictions = 0;
  overall_prediction_error = 0;
  for (size_t i = 0; i < predictions[0][0].size(); ++i) {
    if (samples_oob_count[i] > 0) {
      ++num_predictions;
      predictions[0][0][i] /= (double) samples_oob_count[i];
      double predicted_value = predictions[0][0][i];
      double real_value = data->get(i, dependent_varID);
      overall_prediction_error += (predicted_value - real_value) * (predicted_value - real_value);
    } else {
      predictions[0][0][i] = NAN;
    }
  }

  overall_prediction_error /= (double) num_predictions;
}

// #nocov start
void ForestOnlineRegression::writeOutputInternal() {
  if (verbose_out) {
    *verbose_out << "Tree type:                         " << "Regression" << std::endl;
  }
}

void ForestOnlineRegression::writeConfusionFile() {

// Open confusion file for writing
  std::string filename = output_prefix + ".confusion";
  std::ofstream outfile;
  outfile.open(filename, std::ios::out);
  if (!outfile.good()) {
    throw std::runtime_error("Could not write to confusion file: " + filename + ".");
  }

// Write confusion to file
  outfile << "Overall OOB prediction error (MSE): " << overall_prediction_error << std::endl;

  outfile.close();
  if (verbose_out)
    *verbose_out << "Saved prediction error to file " << filename << "." << std::endl;
}

void ForestOnlineRegression::writePredictionFile() {

// Open prediction file for writing
  std::string filename = output_prefix + ".prediction";
  std::ofstream outfile;
  outfile.open(filename, std::ios::out);
  if (!outfile.good()) {
    throw std::runtime_error("Could not write to prediction file: " + filename + ".");
  }

  // Write
  outfile << "Predictions: " << std::endl;
  if (predict_all) {
    for (size_t k = 0; k < num_trees; ++k) {
      outfile << "Tree " << k << ":" << std::endl;
      for (size_t i = 0; i < predictions.size(); ++i) {
        for (size_t j = 0; j < predictions[i].size(); ++j) {
          outfile << predictions[i][j][k] << std::endl;
        }
      }
      outfile << std::endl;
    }
  } else {
    for (size_t i = 0; i < predictions.size(); ++i) {
      for (size_t j = 0; j < predictions[i].size(); ++j) {
        for (size_t k = 0; k < predictions[i][j].size(); ++k) {
          outfile << predictions[i][j][k] << std::endl;
        }
      }
    }
  }

  if (verbose_out)
    *verbose_out << "Saved predictions to file " << filename << "." << std::endl;
}

void ForestOnlineRegression::saveToFileInternal(std::ofstream& outfile) {

// Write num_variables
  outfile.write((char*) &num_variables, sizeof(num_variables));

// Write treetype
  TreeType treetype = TREE_REGRESSION;
  outfile.write((char*) &treetype, sizeof(treetype));
}

void ForestOnlineRegression::loadFromFileInternal(std::ifstream& infile) {

// Read number of variables
  size_t num_variables_saved;
  infile.read((char*) &num_variables_saved, sizeof(num_variables_saved));

// Read treetype
  TreeType treetype;
  infile.read((char*) &treetype, sizeof(treetype));
  if (treetype != TREE_REGRESSION) {
    throw std::runtime_error("Wrong treetype. Loaded file is not a regression forest.");
  }

  for (size_t i = 0; i < num_trees; ++i) {

    // Read data
    std::vector<std::vector<size_t>> child_nodeIDs;
    readVector2D(child_nodeIDs, infile);
    std::vector<size_t> split_varIDs;
    readVector1D(split_varIDs, infile);
    std::vector<double> split_values;
    readVector1D(split_values, infile);

    // If dependent variable not in test data, change variable IDs accordingly
    if (num_variables_saved > num_variables) {
      for (auto& varID : split_varIDs) {
        if (varID >= dependent_varID) {
          --varID;
        }
      }
    }

    // Create tree
    trees.push_back(std::make_unique<TreeRegression>(child_nodeIDs, split_varIDs, split_values));
  }
}

double ForestOnlineRegression::getTreePrediction(size_t tree_idx, size_t sample_idx) const {
  const auto& tree = dynamic_cast<const TreeRegression&>(*trees[tree_idx]);
  return tree.getPrediction(sample_idx);
}

size_t ForestOnlineRegression::getTreePredictionTerminalNodeID(size_t tree_idx, size_t sample_idx) const {
  const auto& tree = dynamic_cast<const TreeRegression&>(*trees[tree_idx]);
  return tree.getPredictionTerminalNodeID(sample_idx);
}

// #nocov end

}// namespace ranger