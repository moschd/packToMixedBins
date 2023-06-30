#ifndef PACKING_RESULT_EVALUATOR_H
#define PACKING_RESULT_EVALUATOR_H

/**
 * @brief Provides insight in the efficiency of the returned packing result.
 *
 *
 */
class PackingResultEvaluator
{
private:
  Packer packingProcessor_;

public:
  PackingResultEvaluator(Packer aPackingProcessor) : packingProcessor_(aPackingProcessor)
  {
    evaluate();
  };

  void evaluate()
  {
    std::cout << "Running evaluation.\n";

    double totalVolumeOfItems = 0.0;
    for (const auto &consKeyVolumePair : PackingResultEvaluator::packingProcessor_.getContext()->getItemRegister()->getTotalVolumeMap())
    {
      std::cout << "Key:" << consKeyVolumePair.first << " volume: " << consKeyVolumePair.second << "\n";
      totalVolumeOfItems += consKeyVolumePair.second;
    };
    std::cout << "\n";

    const int totalBinsUsed = packingProcessor_.getNumberOfBins();
    const double availableVolumePerBin = packingProcessor_.getContext()->getRequestedBin()->getMaxVolume();
    const double totalAvailableVolume = totalBinsUsed * availableVolumePerBin;
    const double availableVolumeWithOneBinLess = totalAvailableVolume - availableVolumePerBin;

    std::cout << "Total bins used: " << totalBinsUsed << "\n";
    std::cout << "Available volume per bin: " << availableVolumePerBin << "\n";
    std::cout << "Total available volume: " << totalAvailableVolume << "\n";
    std::cout << "---\n";
    std::cout << "Total volume of items to be packed: " << totalVolumeOfItems << "\n";
    std::cout << "Available volume with one bin less: " << availableVolumeWithOneBinLess << "\n";
    std::cout << "---\n";

    if (availableVolumeWithOneBinLess < totalVolumeOfItems)
    {
      std::cout << "OK - This packing request could never be done with less bins.\n";
    }
    else
    {
      std::cout << "WARN - This packing request could possibly be done with less bins.\n";
    }
  };
};

#endif