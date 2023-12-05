designFile = "C:/Users/retin/Desktop/My_Git/RSL10/Altium/AntDig/AntDig/PDNAnalyzer_Output/AntDig/odb.tgz"

powerNets = ["1V8"]

groundNets = ["GND"]

excitation = [
{
"id": "0",
"type": "source",
"power_pins": [ ("C4", "2") ],
"ground_pins": [ ("X1", "4") ],
"voltage": 1.8,
"Rpin": 0,
}
,
{
"id": "1",
"type": "load",
"power_pins": [ ("DA1", "9"), ("DA1", "36") ],
"ground_pins": [ ("DA1", "3"), ("DA1", "8"), ("DA1", "28"), ("DA1", "35"), ("DA1", "42"), ("DA1", "46"), ("DA1", "49") ],
"current": 0.0001,
"Rpin": 3111.11111111111,
}
]


voltage_regulators = []


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
