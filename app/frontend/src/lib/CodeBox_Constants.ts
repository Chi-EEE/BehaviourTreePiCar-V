import { hoverTooltip } from "@codemirror/view";


export const range_0_180 = [
    "000",
    "001",
    "002",
    "003",
    "004",
    "005",
    "006",
    "007",
    "008",
    "009",
    "010",
    "011",
    "012",
    "013",
    "014",
    "015",
    "016",
    "017",
    "018",
    "019",
    "020",
    "021",
    "022",
    "023",
    "024",
    "025",
    "026",
    "027",
    "028",
    "029",
    "030",
    "031",
    "032",
    "033",
    "034",
    "035",
    "036",
    "037",
    "038",
    "039",
    "040",
    "041",
    "042",
    "043",
    "044",
    "045",
    "046",
    "047",
    "048",
    "049",
    "050",
    "051",
    "052",
    "053",
    "054",
    "055",
    "056",
    "057",
    "058",
    "059",
    "060",
    "061",
    "062",
    "063",
    "064",
    "065",
    "066",
    "067",
    "068",
    "069",
    "070",
    "071",
    "072",
    "073",
    "074",
    "075",
    "076",
    "077",
    "078",
    "079",
    "080",
    "081",
    "082",
    "083",
    "084",
    "085",
    "086",
    "087",
    "088",
    "089",
    "090",
    "091",
    "092",
    "093",
    "094",
    "095",
    "096",
    "097",
    "098",
    "099",
    "100",
    "101",
    "102",
    "103",
    "104",
    "105",
    "106",
    "107",
    "108",
    "109",
    "110",
    "111",
    "112",
    "113",
    "114",
    "115",
    "116",
    "117",
    "118",
    "119",
    "120",
    "121",
    "122",
    "123",
    "124",
    "125",
    "126",
    "127",
    "128",
    "129",
    "130",
    "131",
    "132",
    "133",
    "134",
    "135",
    "136",
    "137",
    "138",
    "139",
    "140",
    "141",
    "142",
    "143",
    "144",
    "145",
    "146",
    "147",
    "148",
    "149",
    "150",
    "151",
    "152",
    "153",
    "154",
    "155",
    "156",
    "157",
    "158",
    "159",
    "160",
    "161",
    "162",
    "163",
    "164",
    "165",
    "166",
    "167",
    "168",
    "169",
    "170",
    "171",
    "172",
    "173",
    "174",
    "175",
    "176",
    "177",
    "178",
    "179",
    "180",
];

export const range_0_100 = [
    "000",
    "001",
    "002",
    "003",
    "004",
    "005",
    "006",
    "007",
    "008",
    "009",
    "010",
    "011",
    "012",
    "013",
    "014",
    "015",
    "016",
    "017",
    "018",
    "019",
    "020",
    "021",
    "022",
    "023",
    "024",
    "025",
    "026",
    "027",
    "028",
    "029",
    "030",
    "031",
    "032",
    "033",
    "034",
    "035",
    "036",
    "037",
    "038",
    "039",
    "040",
    "041",
    "042",
    "043",
    "044",
    "045",
    "046",
    "047",
    "048",
    "049",
    "050",
    "051",
    "052",
    "053",
    "054",
    "055",
    "056",
    "057",
    "058",
    "059",
    "060",
    "061",
    "062",
    "063",
    "064",
    "065",
    "066",
    "067",
    "068",
    "069",
    "070",
    "071",
    "072",
    "073",
    "074",
    "075",
    "076",
    "077",
    "078",
    "079",
    "080",
    "081",
    "082",
    "083",
    "084",
    "085",
    "086",
    "087",
    "088",
    "089",
    "090",
    "091",
    "092",
    "093",
    "094",
    "095",
    "096",
    "097",
    "098",
    "099",
    "100",
]

export const wheel_type = ["Left", "Right", "Both"]

export const servo_type = ["FrontWheels", "CameraServo1", "CameraServo2"]

export const common_nodes = [
    // Composite
    "Selector",
    "Sequence",

    // Decorator
    "Invert",
    "Repeat",

    // Leaf
    "Fail",
    "Succeed",
    "Task",
    "UseRoot",

    // Task Nodes
    "Action:CountdownWait",
    "Action:PauseExecution",

    "Action:Log",
    "Action:Error",

    "Action:SetAngle",
    "Action:SetSpeed",
    "Action:SetWheelDirection",

    "Condition:NearbyPoints",
];

export const xml_schema = {
    elements: [
        {
            name: "BehaviourTree",
            top: true,
            children: ["Root"],
            completion: { type: "keyword" },
        },
        {
            name: "Root",
            attributes: ["id"],
            children: common_nodes,
            completion: { type: "keyword" },
        },
        // Composite
        {
            name: "Selector",
            children: common_nodes,
            completion: { type: "keyword" },
        },
        {
            name: "Sequence",
            children: common_nodes,
            completion: { type: "keyword" },
        },
        {
            name: "Random",
            children: common_nodes,
            completion: { type: "keyword" },
        },
        // Decorator
        {
            name: "Invert",
            children: common_nodes,
            completion: { type: "keyword" },
        },
        {
            name: "Repeat",
            children: common_nodes,
            attributes: [
                "count",
                {
                    name: "break_on_fail",
                    values: ["true", "false"],
                    completion: { type: "keyword" },
                }
            ],
            completion: { type: "keyword" },
        },

        // Leaf
        {
            name: "Fail",
            completion: { type: "keyword" },
        },
        {
            name: "Succeed",
            completion: { type: "keyword" },
        },
        {
            name: "UseRoot",
            attributes: ["id"],
            completion: { type: "keyword" },
        },
        {
            name: "Action:CountdownWait",
            attributes: [
                "ms",
                {
                    name: "reset_on_non_consecutive_tick",
                    values: ["true", "false"],
                    completion: { type: "keyword" },
                }
            ],
            completion: { type: "keyword" },
        },
        {
            name: "Action:PauseExecution",
            attributes: ["ms"],
            completion: { type: "keyword" },
        },
        {
            name: "Action:Log",
            attributes: ["text"],
            completion: { type: "keyword" },
        },
        {
            name: "Action:Error",
            attributes: ["text"],
            completion: { type: "keyword" },
        },
        {
            name: "Action:SetAngle",
            attributes: [
                {
                    name: "servo_type",
                    values: servo_type,
                    completion: { type: "keyword" },
                },
                {
                    name: "angle",
                    values: range_0_180,
                    completion: { type: "keyword" },
                },
            ],
            completion: { type: "keyword" },
        },
        {
            name: "Action:SetSpeed",
            attributes: [
                {
                    name: "wheel_type",
                    values: wheel_type,
                    completion: { type: "keyword" },
                },
                {
                    name: "speed",
                    values: range_0_100,
                    completion: { type: "keyword" },
                },
            ],
            completion: { type: "keyword" },
        },
        {
            name: "Action:SetWheelDirection",
            attributes: [
                {
                    name: "wheel_type",
                    values: wheel_type,
                    completion: { type: "keyword" },
                },
                {
                    name: "direction_type",
                    values: ["Forward", "Backward"],
                    completion: { type: "keyword" },
                },
            ],
            completion: { type: "keyword" },
        },
        {
            name: "Condition:NearbyPoints",
            attributes: [
                {
                    name: "min_angle",
                    values: range_0_180,
                    completion: { type: "keyword" },
                },
                {
                    name: "max_angle",
                    values: range_0_180,
                    completion: { type: "keyword" },
                },
                "distance",
            ],
            completion: { type: "keyword" },
        },
    ],
    attributes: [{ name: "name", global: true }],
};

const node_information_list = new Map<string, string>([
    ["Root", "Start of every node tree"],
    ["Selector", "Runs children until one succeeds"],
    ["Sequence", "Runs children until one fails"],

    ["UseRoot", "Goes to the given root node id"],
    ["Invert", "Inverts the result of the child"],
    ["Repeat", "Repeats the child given the amount of times"],

    ["Fail", "Always fails"],
    ["Succeed", "Always succeeds"],

    ["Action:CountdownWait", "Waits for the given amount of time"],
    ["Action:PauseExecution", "Pauses the execution for the given amount of time"],
    ["Action:Log", "Logs the given message"],
    ["Action:Error", "Logs errors the given message"],

    ["Action:SetAngle", "Turns the car by the given angle"],
    ["Action:SetSpeed", "Moves the car by the given speed"],
    ["Action:SetWheelDirection", "Sets the direction of the car"],
    ["Action:Stop", "Stops the car from moving"],

    ["Condition:NearbyPoints", "Checks if there are nearby points"],
]);

// Modified code from: https://codemirror.net/examples/tooltip/
export const node_hover = hoverTooltip((view, pos, side) => {
    const { from, to, text } = view.state.doc.lineAt(pos);
    let start = pos,
        end = pos;
    while (start > from && /[\w|:]/.test(text[start - from - 1])) start--;
    while (end < to && /[\w|:]/.test(text[end - from])) end++;
    if ((start == pos && side < 0) || (end == pos && side > 0)) return null;
    const highlighted_node = text.slice(start - from, end - from);
    const node_information = node_information_list.get(highlighted_node);
    if (!node_information) return null;
    return {
        pos: start,
        end,
        above: true,
        create(_) {
            const dom = document.createElement("div");
            dom.textContent = node_information;
            return { dom };
        },
    };
});