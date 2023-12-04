designFile = "C:/Users/retin/Desktop/My_Git/RSL10/Altium/RSL10/PDNAnalyzer_Output/Analog/odb.tgz"

powerNets = ["1V8", "NetR9_1"]

groundNets = ["GND"]

excitation = [
{
"id": "0",
"type": "source",
"power_pins": [ ("C7", "2") ],
"ground_pins": [ ("C7", "1") ],
"voltage": 1.8,
"Rpin": 0,
}
,
{
"id": "1",
"type": "load",
"power_pins": [ ("DD1", "5") ],
"ground_pins": [ ("DD1", "2") ],
"current": 0.0001,
"Rpin": 1000,
}
,
{
"id": "2",
"type": "load",
"power_pins": [ ("DD2", "5") ],
"ground_pins": [ ("DD2", "2") ],
"current": 0.0001,
"Rpin": 1000,
}
,
{
"id": "3",
"type": "load",
"power_pins": [ ("DD3", "5") ],
"ground_pins": [ ("DD3", "2") ],
"current": 0.0001,
"Rpin": 1000,
}
,
{
"id": "4",
"type": "load",
"power_pins": [ ("DA2", "9"), ("DA2", "1") ],
"ground_pins": [ ("DA2", "10") ],
"current": 0.0001,
"Rpin": 1333.33333333333,
}
,
{
"id": "5",
"type": "load",
"power_pins": [ ("VD2", "A") ],
"ground_pins": [ ("VT1", "2") ],
"current": 0.05,
"Rpin": 2,
}
]


voltage_regulators = [
{
"id": "6",
"type": "linear",

"in": [ ("R9", "2") ],
"out": [ ("VD2", "A") ],
"ref": [],

"v2": 0,
"i1": 0,
"Ro": 1E-06,
"Rpin": 5,
}
]


# Resistors / Inductors

passives = []


# Material Properties:

tech = [

        {'name': 'TOP_SOLDER', 'DielectricConstant': 3.5, 'Thickness': 1.016E-05},
        {'name': 'TOP_LAYER', 'Conductivity': 47000000, 'Thickness': 1.8E-05},
        {'name': 'SUBSTRATE-1', 'DielectricConstant': 4.8, 'Thickness': 0.001},
        {'name': 'BOTTOM_LAYER', 'Conductivity': 47000000, 'Thickness': 1.8E-05},
        {'name': 'BOTTOM_SOLDER', 'DielectricConstant': 3.5, 'Thickness': 1.016E-05}

       ]

special_settings = {'removecutoutsize' : 7.8 }


plating_thickness = 0.7
finished_hole_diameters = False
