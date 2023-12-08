#include "RPLidar.h"

// Made with the help of ChatGPT

/*
auto lidar = RPLidar("COM3");

auto info = lidar.get_info();
std::cout << fmt::format("model: {}, firmware: ({}, {}), hardware: {}, serialnumber: {}\n", info.model, info.firmware.first, info.firmware.second, info.hardware, info.serialNumber);

auto health = lidar.get_health();
std::cout << fmt::format("({}, {})\n", health.status, health.errorCode);

std::function<std::vector<Measure>()> scanGenerator = lidar.iter_scans();
for (int i = 0; i < 10; i++)
{
    std::vector<Measure> scan = scanGenerator();
    std::cout << "Got " << scan.size() << " Measures!\n";
    for (const Measure &measure : scan)
    {
        // Access individual measurements in the scan
        bool newScan = measure.newScan;
        int quality = measure.quality;
        float angle = measure.angle;
        float distance = measure.distance;
    }
}

lidar.stop();
lidar.stop_motor();
lidar.disconnect();

*/

/**
 * @brief Initilize RPLidar object for communicating with the sensor
 * 
 * @param port Serial port name to which sensor is connected
 * @param baudrate Baudrate for serial connection (the default is 115200)
 */
RPLidar::RPLidar(const std::string &port, uint32_t baudrate)
{
    this->port = port;
    this->baudrate = baudrate;
    try
    {
        this->_serial = std::make_unique<serial::Serial>(port, baudrate, serial::Timeout(1000U));
        std::cout << "Opened serial: " << _serial->isOpen() << '\n';
    }
    catch (std::exception &e)
    {
        std::cout << "Unable to open serial: " << e.what() << '\n';
    }
}

RPLidar::~RPLidar()
{
}

void RPLidar::disconnect()
{
    if (!this->_serial->isOpen())
    {
        return;
    }
    this->_serial->close();
}

void RPLidar::_set_pwm(int pwm)
{
    std::string payload;
    payload.push_back(static_cast<char>(pwm & 0xFF));
    payload.push_back(static_cast<char>((pwm >> 8) & 0xFF));
    this->_send_payload_cmd(SET_PWM_BYTE, payload);
}

void RPLidar::set_motor_speed(int pwm)
{
    if (0 <= pwm && pwm <= MAX_MOTOR_PWM)
    {
        std::abort();
    }
    this->_motor_speed = pwm;
    if (this->motor_running)
        this->_set_pwm(this->_motor_speed);
}

/**
 * @brief Starts sensor motor
 * 
 */
void RPLidar::start_motor()
{
    spdlog::info("Starting motor");
    // For A1
    this->_serial->setDTR(false);

    // For A2
    this->_set_pwm(this->_motor_speed);
    this->motor_running = true;
}

/**
 * @brief Stops sensor motor
 * 
 */
void RPLidar::stop_motor()
{
    spdlog::info("Stopping motor");
    // For A2
    this->_set_pwm(0);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // For A1
    this->_serial->setDTR(true);
    this->motor_running = false;
}

/**
 * @brief Sends `cmd` command with `payload` to the sensor
 * 
 * @param cmd 
 * @param payload 
 */
void RPLidar::_send_payload_cmd(uint8_t cmd, const std::string &payload)
{
    // Calculate the size
    uint8_t size = static_cast<uint8_t>(payload.size());

    // Construct the request string
    std::string req;
    req += static_cast<char>(SYNC_BYTE);
    req += static_cast<char>(cmd);
    req += static_cast<char>(size);
    req += payload;

    // Calculate the checksum
    uint8_t checksum = 0;
    for (const char &c : req)
    {
        checksum ^= static_cast<uint8_t>(c);
    }

    req += static_cast<char>(checksum);

    this->_serial->write(req);
    spdlog::debug("Command sent: {}", spdlog::to_hex(req));
}

/**
 * @brief Sends `cmd` command to the sensor
 * 
 * @param cmd 
 */
void RPLidar::_send_cmd(uint8_t cmd)
{
    std::string req;
    req.push_back(SYNC_BYTE);
    req.push_back(cmd);
    this->_serial->write(req);
    spdlog::debug("Command sent: {}", spdlog::to_hex(req));
}

/**
 * @brief Reads descriptor packet
 * 
 * @return std::tuple<uint8_t, bool, uint8_t> 
 */
std::tuple<uint8_t, bool, uint8_t> RPLidar::_read_descriptor()
{
    // Read descriptor packet
    std::vector<uint8_t> descriptor(DESCRIPTOR_LEN);
    this->_serial->read(descriptor.data(), DESCRIPTOR_LEN);
    spdlog::debug("Received descriptor: {}", spdlog::to_hex(descriptor));

    if (descriptor.size() != DESCRIPTOR_LEN)
    {
        throw std::runtime_error(fmt::format("Descriptor length mismatch : {}", descriptor.size()));
    }
    else if (descriptor[0] != SYNC_BYTE || descriptor[1] != SYNC_BYTE2)
    {
        throw std::runtime_error(fmt::format("Incorrect descriptor starting bytes : {} | {}", descriptor[0], descriptor[1]));
    }

    bool isSingle = descriptor[5] == 0;
    return std::make_tuple(descriptor[2], isSingle, descriptor[6]);
}

/**
 * @brief Reads response packet with length of `dsize` bytes
 * 
 * @param dsize 
 * @return std::vector<uint8_t> 
 */
std::vector<uint8_t> RPLidar::_read_response(int dsize)
{
    spdlog::debug("Trying to read response: {} bytes", dsize);

    std::vector<uint8_t> data;
    data.reserve(dsize);

    while (this->_serial->available() < dsize)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    this->_serial->read(data, dsize);

    spdlog::debug("Received data: {}", spdlog::to_hex(data));
    return data;
}

std::string convertToHexString(uint8_t value)
{
    // Convert a uint8_t to a hexadecimal string
    std::stringstream stream;
    stream << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(value);
    return stream.str();
}

/**
 * @brief Get device information
 * 
 * @return DeviceInfo 
 */
DeviceInfo RPLidar::get_info()
{
    // Check if there's data in the buffer
    if (this->_serial->available() > 0)
    {
        throw std::runtime_error("Data in buffer, you can't have info! Run flush() to empty the buffer.");
    }
    this->_send_cmd(GET_INFO_BYTE);

    uint8_t dsize;
    bool isSingle;
    uint8_t dtype;
    std::tie(dsize, isSingle, dtype) = this->_read_descriptor();

    // Check response properties
    if (dsize != INFO_LEN)
    {
        throw std::runtime_error("Wrong get_info reply length");
    }
    if (!isSingle)
    {
        throw std::runtime_error("Not a single response mode");
    }
    if (dtype != INFO_TYPE)
    {
        throw std::runtime_error("Wrong response data type");
    }

    // Read the response
    std::vector<uint8_t> raw = this->_read_response(dsize);

    // Convert serial number to a hex string
    std::string serialNumber;
    for (size_t i = 4; i < raw.size(); ++i)
    {
        serialNumber += convertToHexString(raw[i]);
    }

    // Construct the device info struct
    DeviceInfo info;
    info.model = raw[0];
    info.firmware.first = raw[2];
    info.firmware.second = raw[1];
    info.hardware = raw[3];
    info.serialNumber = serialNumber;

    return info;
}

/**
 * @brief Get device health state. When the core system detects some
        potential risk that may cause hardware failure in the future,
        the returned status value will be 'Warning'. But sensor can still work
        as normal. When sensor is in the Protection Stop state, the returned
        status value will be 'Error'. In case of warning or error statuses
        non-zero error code will be returned.
 * 
 * @return HealthInfo 
 */
HealthInfo RPLidar::get_health()
{
    // Check if there's data in the buffer
    if (this->_serial->available() > 0)
    {
        throw std::runtime_error("Data in buffer, you can't get health info! Run cleanInput() to empty the buffer.");
    }
    spdlog::info("Asking for health");

    this->_send_cmd(GET_HEALTH_BYTE);

    // Read the descriptor
    uint8_t dsize;
    bool isSingle;
    uint8_t dtype;
    std::tie(dsize, isSingle, dtype) = this->_read_descriptor();

    // Check response properties
    if (dsize != HEALTH_LEN)
    {
        throw std::runtime_error("Wrong get_health reply length");
    }
    if (!isSingle)
    {
        throw std::runtime_error("Not a single response mode");
    }
    if (dtype != HEALTH_TYPE)
    {
        throw std::runtime_error("Wrong response data type");
    }

    // Read the response
    std::vector<uint8_t> raw = this->_read_response(dsize);

    // Extract status and error code
    std::string status = HEALTH_STATUSES[raw[0]];
    int errorCode = (static_cast<int>(raw[1]) << 8) + static_cast<int>(raw[2]);

    return {status, errorCode};
}

/**
 * @brief 
 * Clean input buffer by reading all available data
 */
void RPLidar::clean_input()
{
    if (this->scanning.currently_scanning)
    {
        throw std::runtime_error("Cleaning not allowed during scanning process active!");
    }
    this->_serial->flushInput();
    this->express_trame = 32;
    this->express_data = nullptr;
}

/**
 * @brief 
 * Stops scanning process, disables laser diode and the measurement
        system, moves sensor to the idle state.
 */
void RPLidar::stop()
{
    spdlog::info("Stopping scanning");
    this->_send_cmd(STOP_BYTE);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    this->scanning.currently_scanning = false;
    this->clean_input();
}

/**
 * @brief Start the scanning process
 * 
 * @param scanType NORMAL, FORCE or EXPRESS
 */
void RPLidar::start(ScanType scanType = NORMAL)
{
    if (this->scanning.currently_scanning)
    {
        throw std::runtime_error("Scanning already running!");
    }

    HealthInfo healthInfo = this->get_health();
    std::string status = healthInfo.status;
    int errorCode = healthInfo.errorCode;
    spdlog::debug("Health status: {} [{}}]", status, errorCode);

    if (status == "Error")
    {
        this->reset();
        healthInfo = this->get_health();
        status = healthInfo.status;
        errorCode = healthInfo.errorCode;
        if (status == "Error")
        {
            throw std::runtime_error("RPLidar hardware failure. Error code: " + std::to_string(errorCode));
        }
    }
    else if (status == "Warning")
    {
        spdlog::warn("Warning sensor status detected! Error code: {}", errorCode);
    }

    uint8_t cmd = SCAN_TYPE[scanType]["byte"];

    spdlog::warn("starting scan process in {} mode", scanType);

    if (scanType == EXPRESS)
    {
        // Adjust this part according to your payload format
        std::string payload = "\x00\x00\x00\x00\x00";
        this->_send_payload_cmd(cmd, payload);
    }
    else
    {
        this->_send_cmd(cmd);
    }

    uint8_t dsize;
    bool isSingle;
    uint8_t dtype;
    std::tie(dsize, isSingle, dtype) = this->_read_descriptor();

    if (dsize != SCAN_TYPE[scanType]["size"])
    {
        throw std::runtime_error("Wrong get_info reply length");
    }
    if (isSingle)
    {
        throw std::runtime_error("Not a multiple response mode");
    }
    if (dtype != SCAN_TYPE[scanType]["response"])
    {
        throw std::runtime_error("Wrong response data type");
    }

    scanning = {true, dsize, scanType};
}

/**
 * @brief Resets sensor core, reverting it to a similar state as it has
        just been powered up.
 * 
 */
void RPLidar::reset()
{
    spdlog::info("Resetting the sensor");
    this->_send_cmd(RESET_BYTE);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    this->clean_input();
}

Measure _process_scan(const std::vector<uint8_t> &raw)
{
    Measure measurementData;

    bool newScan = static_cast<bool>(raw[0] & 0b1);
    bool inversedNewScan = static_cast<bool>((raw[0] >> 1) & 0b1);
    int quality = static_cast<int>(raw[0] >> 2);

    if (newScan == inversedNewScan)
    {
        throw std::runtime_error("New scan flags mismatch");
    }

    int checkBit = static_cast<int>(raw[1] & 0b1);
    if (checkBit != 1)
    {
        throw std::runtime_error("Check bit not equal to 1");
    }

    int anglePart1 = static_cast<int>(raw[1] >> 1);
    int anglePart2 = static_cast<int>(raw[2]) << 7;
    float angle = (anglePart1 + anglePart2) / 64.0;

    int distancePart1 = static_cast<int>(raw[3]);
    int distancePart2 = static_cast<int>(raw[4]) << 8;
    float distance = (distancePart1 + distancePart2) / 4.0;

    measurementData.newScan = newScan;
    measurementData.quality = quality;
    measurementData.angle = angle;
    measurementData.distance = distance;

    return measurementData;
}

Measure _process_express_scan(std::unique_ptr<ExpressPacket> &data, float newAngle, int trame)
{
    Measure measurementData;

    bool newScan = (newAngle < data->start_angle) && (trame == 1);

    float angle = std::fmod((data->start_angle + ((newAngle - data->start_angle) / 32 * trame - data->angle[trame - 1])), 360);
    float distance = data->distance[trame - 1];

    measurementData.newScan = newScan;
    measurementData.quality = 0; // Replace this with your actual quality value
    measurementData.angle = angle;
    measurementData.distance = distance;

    return measurementData;
}

/**
 * @brief Iterate over measures. Note that consumer must be fast enough,
        otherwise data will be accumulated inside buffer and consumer will get
        data with increasing lag.
 * 
 * @param scanType 
 * @param maxBufMeas int or False if you want unlimited buffer
            Maximum number of bytes to be stored inside the buffer. Once
            numbe exceeds this limit buffer will be emptied out.
 * @return std::function<Measure()> 
 */
std::function<Measure()> RPLidar::iter_measures(ScanType scanType, int maxBufMeas)
{
    this->start_motor();

    if (!this->scanning.currently_scanning)
    {
        this->start(scanType);
    }

    // Define a lambda function to generate measures
    auto generator = [this, scanType, maxBufMeas]() -> Measure
    {
        while (true)
        {
            int dsize = scanning.dsize;

            if (maxBufMeas)
            {
                int dataInBuf = this->_serial->available();
                if (dataInBuf > maxBufMeas)
                {
                    spdlog::warn(
                        "Too many bytes in the input buffer: {}/{}. \n"
                        "Cleaning buffer...",
                        dataInBuf, maxBufMeas);
                    this->stop();
                    this->start(scanning.type);
                }
            }

            if (scanType == NORMAL)
            {
                std::vector<uint8_t> raw = this->_read_response(dsize);
                Measure measure = _process_scan(raw);
                return measure;
            }
            else if (scanType == EXPRESS)
            {
                if (this->express_trame == 32)
                {
                    this->express_trame = 0;

                    if (this->express_data == nullptr)
                    {
                        spdlog::debug("reading first time bytes");
                        this->express_data = std::make_unique<ExpressPacket>(ExpressPacket(this->_read_response(dsize)));
                    }

                    this->express_old_data = std::move(this->express_data);
                    spdlog::debug("set old_data with start_angle {}", this->express_old_data->start_angle);
                    this->express_data = std::make_unique<ExpressPacket>(ExpressPacket(this->_read_response(dsize)));
                    spdlog::debug("set new_data with start_angle {}", this->express_data->start_angle);
                }
                this->express_trame++;
                spdlog::debug("process scan of frame %d with angle : \n"
                                "%f and angle new : %f", this->express_trame,
                                this->express_old_data->start_angle,
                                this->express_data->start_angle);
                Measure measure = _process_express_scan(this->express_old_data, this->express_data->start_angle, this->express_trame);
                return measure;
            }
        }
    };

    return generator;
}

/**
 * @brief Iterate over scans. Note that consumer must be fast enough,
        otherwise data will be accumulated inside buffer and consumer will get
        data with increasing lag.
 * 
 * @param scanType 
 * @param maxBufMeas Maximum number of measures to be stored inside the buffer. Once
            numbe exceeds this limit buffer will be emptied out.
 * @param minLen Minimum number of measures in the scan for it to be yelded.
 * @return std::function<std::vector<Measure>()> 
 */
std::function<std::vector<Measure>()> RPLidar::iter_scans(ScanType scanType, int maxBufMeas, int minLen)
{
    std::vector<Measure> scanList;
    std::function<Measure()> measureIterator = this->iter_measures(scanType, maxBufMeas);

    // Define a lambda function to generate scans
    auto scanGenerator = [this, scanType, maxBufMeas, minLen, measureIterator]() -> std::vector<Measure>
    {
        std::vector<Measure> scanList;
        for (;;)
        {
            Measure measure = measureIterator();
            bool newScan = measure.newScan;
            if (newScan)
            {
                if (scanList.size() > minLen)
                {
                    return scanList;
                }
                scanList.clear();
            }
            if (measure.distance > 0)
            {
                scanList.push_back(measure);
            }
        }
    };
    return scanGenerator;
}
