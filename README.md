# packToMixedBins
Variant of the packToBin algorithm except it accepts multiple bins.

Largely a copy, adds a bin selector object which orchestrates bin selection.

## TODO 
Write a fully new object which can hold packed bins.


input supports 4 decimals



BinComposer:


- requestedBinObjects
- items

Creat packers by iterating over the requestedBins

check if there are items which only fit in a particular bin.
if yes:
    pack those items and fill the bin as much as possible.

call packToBin for all bins.
    if all bigger than 1
        choose the bin with the least packed bins, from the packed bins, choose the most efficiently packed one.
        call function again.
    if there is 1 with 1
        pack it
    if there are multiple with 1:
        pack smallest one