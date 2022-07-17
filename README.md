# packToBin
Algorithm to place multiple rectangles in another rectangle in the most desired way possible.

## Input
Make a function call to <i>packToBinAlgorhitm</i>.

### Parameters
| Name | Data Type | Default Value |
|----------|-------------|-------------|
| incomingJson |  char *  | NA |
| includeBins |  bool  | 1 |
| includeItems |  bool  | 1 |
| itemDimensionsAfter |  bool  | 1 |
| jsonPrecision |  int | 5 |
| gravityStrength |  double | 0.0 |

#### incomingJson
This is the parameter which supplies the JSON.
#### includeBins
This parameter indicates if the bins should be included in the result. This is used if you are only interested in high level final result such as total weight/volume utilization.
#### includeItems
This parameter indicates if the items should be included in the result. This is used if you are not interested in the individual item details inside each bin.
#### itemDimensionsAfter
This parameter indicates if the dimensions of the items in the response should be adjusted for the way that they are rotated.
#### jsonPrecision
This parameter controls the decimal precision in the response body.
#### gravityStrength
This parameter controls the level of gravity that needs to be taken into account. It is basically surface area expressed in percentage of the item to be placed that should be supported in order to be considered a valid item placement.

### Body
```json
{
	"bin": {
		"type": "Pallet",
		"maxWeight": 1500,
		"width": 1.2,
		"height": 1.65,
		"depth": 0.8
	},
	"items": [{
			"id": "DTM001",
			"width": 0.1,
			"height": 1.62,
			"depth": 0.2,
			"weight": 10,
			"allowedRotations": "012345"
		},
		{
			"id": "DTM002",
			"width": 0.29,
			"height": 0.32,
			"depth": 0.49,
			"weight": 10,
			"allowedRotations": "012345"
		}
    ]
}
```

## Output
<i>packToBinAlgorhitm</i> returns a <i>const char *</i>.

### Body
```json
{
    "binDetails": {
        "type": "Pallet",
        "maxDepth": 0.8,
        "maxHeight": 1.65,
        "maxVolume": 1.584,
        "maxWeight": 1500,
        "maxWidth": 1.2
    },
    "packedBins": [
        {
            "actualVolume": 0.907872,
            "actualVolumeUtil": 57.31515,
            "actualWeight": 1500,
            "actualWeightUtil": 100,
            "binName": "Pallet-1",
            "fittedItems": [
                {
                    "allowedRotations": "01",
                    "depth": 0.7,
                    "height": 1.12,
                    "id": "DTM001",
                    "rotationType": 0,
                    "rotationTypeDescription": "No box rotation",
                    "volume": 0.8624,
                    "weight": 1000,
                    "width": 1.1,
                    "xCoordinate": 0,
                    "yCoordinate": 0,
                    "zCoordinate": 0
                },
                {
                    "allowedRotations": "012345",
                    "depth": 0.49,
                    "height": 0.32,
                    "id": "DTM002",
                    "rotationType": 0,
                    "rotationTypeDescription": "No box rotation",
                    "volume": 0.045472,
                    "weight": 500,
                    "width": 0.29,
                    "xCoordinate": 0,
                    "yCoordinate": 0,
                    "zCoordinate": 1.12
                }
            ],
            "nrOfItems": 2
        }
    ],
    "requiredNrOfBins": 1,
    "totalVolumeUtil": 57.31515,
    "totalWeightUtil": 100.0
}
```
---

# Inner Workings

1. main.cpp
Driver Program.

2. packer.h
Class that represents the packing process for all items across all bins.

3. bin.h
Class that represents a individual bin.

4. item.h
Class that represents a individual item.

5. 3r-kd-tree.h
# 3R kd-tree.
The packToBin algorithm stores its geospatial data a kd-tree datastructure. The kd-tree contains 3 dimensions which are the cartesian coordinates of each item that is placed in the bin.

## Tree generation
The tree is pre-generated with a certain depth, each pre generated branch has a calculated partitioning point where each paritioning point is the middle of the axis to be split.
ie a bin of {1, 1, 1}

## Tree data insertion
Item keys are only stored in the leaves of the tree.

## Number of Leaves
The number of leaves on the pre-generated kd-tree is determined by the maxDepth parameter.
| Max Depth   |      Leaves   |
|----------|-------------|
| 4 |  32  |
| 5 |  64  |
| 6 |  128  |
| 7 |  256  |
| 8 |  512 |
| 9 |  1024  |
| 10| 2048 |

The max depth parameter is a calculated value based on a estimation of the number of items that will go into the bin. This will allow the tree to grow accordingly and leading to improved performance.

6. gravity.h
Class that hold gravity logic.

7. itemregister.h
Register to contain all items to prevent duplicating a item objects.

8. outgoingJsonBuilder.h
Holds the logic for building the outgoing JSON.

9. miscfunctions.h
Holds multiple general purpose functions.

10. ~~binsection.h~~
Deprecated.
Class that represented a virtual bin inside a bigger bin.

11. ~~partitioner.h~~
Deprecated.
Class that was used to parition bins into multiple bin sections.
