#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <mutex>
#include <memory>
#include <unordered_map>
#include <chrono>
#include <thread>

// --- Enums and Base Classes ---

// Enum for vehicle types
enum class VehicleType {
    Motorcycle,
    Car,
    Truck
};

// Enum for parking spot sizes
enum class SpotSize {
    Motorcycle,
    Compact,
    Large
};

// Abstract base class for vehicles
class Vehicle {
public:
    virtual ~Vehicle() = default;
    virtual VehicleType getType() const = 0;
    virtual SpotSize getSize() const = 0;
};

// Derived classes for specific vehicles
class Motorcycle : public Vehicle {
public:
    VehicleType getType() const override { return VehicleType::Motorcycle; }
    SpotSize getSize() const override { return SpotSize::Motorcycle; }
};

class Car : public Vehicle {
public:
    VehicleType getType() const override { return VehicleType::Car; }
    SpotSize getSize() const override { return SpotSize::Compact; }
};

class Truck : public Vehicle {
public:
    VehicleType getType() const override { return VehicleType::Truck; }
    SpotSize getSize() const override { return SpotSize::Large; }
};

// --- Ticket and Payment System ---

// Class to hold a vehicle's parking information
class Ticket {
public:
    int spotId;
    std::string vehicleId;
    std::chrono::system_clock::time_point entryTime;

    Ticket() : spotId(-1), vehicleId(""), entryTime(std::chrono::system_clock::now()) {} // Default constructor

    Ticket(int _spotId, const std::string& _vehicleId)
        : spotId(_spotId), vehicleId(_vehicleId), entryTime(std::chrono::system_clock::now()) {}
};

// Enum for different payment methods
enum class PaymentMethod {
    CreditCard,
    DebitCard,
    Cash,
    MobileApp
};

// Class to handle fee calculation and payment processing
class PaymentProcessor {
private:
    double hourlyRate;

public:
    PaymentProcessor(double rate) : hourlyRate(rate) {}

    // Calculates the parking fee based on duration
    double calculateFee(const Ticket& ticket) {
        auto now = std::chrono::system_clock::now();
        std::chrono::duration<double> duration = now - ticket.entryTime;
        double hours = duration.count() / 3600.0;
        return std::max(0.0, hours * hourlyRate); // Ensure fee is non-negative
    }

    // Simulates payment processing for different methods
    bool processPayment(double amount, PaymentMethod method) {
        std::string methodStr;
        switch (method) {
            case PaymentMethod::CreditCard: methodStr = "Credit Card"; break;
            case PaymentMethod::DebitCard:  methodStr = "Debit Card"; break;
            case PaymentMethod::Cash:       methodStr = "Cash"; break;
            case PaymentMethod::MobileApp:  methodStr = "Mobile App"; break;
        }

        std::cout << "Processing payment of $" << amount << " using " << methodStr << "..." << std::endl;
        
        if (amount > 0) {
            std::cout << "Payment successful!" << std::endl;
            return true;
        }
        std::cout << "Payment failed. Amount must be positive." << std::endl;
        return false;
    }
};

// --- Core Parking System Classes ---

// Represents an individual parking spot
class ParkingSpot {
private:
    int id;
    SpotSize size;
    bool isOccupied;
    // unique_ptr ensures exclusive ownership of the vehicle object
    std::unique_ptr<Vehicle> parkedVehicle;

public:
    ParkingSpot(int _id, SpotSize _size)
        : id(_id), size(_size), isOccupied(false) {}

    // Method to park a vehicle. The unique_ptr is moved, transferring ownership.
    bool parkVehicle(std::unique_ptr<Vehicle> vehicle) {
        if (!isOccupied && vehicle->getSize() <= size) {
            parkedVehicle = std::move(vehicle);
            isOccupied = true;
            return true;
        }
        return false;
    }

    // Unparks the vehicle by releasing the unique_ptr and updating the state
    void unparkVehicle() {
        parkedVehicle.reset();
        isOccupied = false;
    }

    bool isAvailable() const { return !isOccupied; }
    int getId() const { return id; }
    SpotSize getSize() const { return size; }
};

// Represents a single level of the parking lot
class ParkingLotLevel {
private:
    int levelNumber;
    // Map of available spots, categorized by size
    std::map<SpotSize, std::vector<std::unique_ptr<ParkingSpot>>> spots;
    // Mutex for thread-safe access to the spots
    std::mutex mtx;

public:
    ParkingLotLevel(int number) : levelNumber(number) {}

    // Adds a new parking spot to the level
    void addSpot(std::unique_ptr<ParkingSpot> spot) {
        spots[spot->getSize()].push_back(std::move(spot));
    }

    // Finds an available spot and parks the vehicle, transferring ownership
    std::unique_ptr<ParkingSpot> findAndPark(std::unique_ptr<Vehicle> vehicle) {
        std::lock_guard<std::mutex> lock(mtx);
        SpotSize vehicleSize = vehicle->getSize();

        // Check for a spot of the same size or larger
        for (auto& pair : spots) {
            if (pair.first >= vehicleSize) {
                for (auto& spot : pair.second) {
                    if (spot->isAvailable()) {
                        spot->parkVehicle(std::move(vehicle));
                        return std::move(spot);
                    }
                }
            }
        }
        return nullptr; // No spot available
    }

    // Unparks a vehicle given a spot ID
    bool unpark(int spotId) {
        std::lock_guard<std::mutex> lock(mtx);
        for (auto& pair : spots) {
            for (auto& spot : pair.second) {
                if (spot->getId() == spotId && !spot->isAvailable()) {
                    spot->unparkVehicle();
                    return true;
                }
            }
        }
        return false;
    }

    int getAvailableSpots(SpotSize size) const {
        int count = 0;
        if (spots.count(size)) {
            for (const auto& spot : spots.at(size)) {
                if (spot->isAvailable()) {
                    count++;
                }
            }
        }
        return count;
    }

    int getLevelNumber() const { return levelNumber; }
};

// The main class for the entire parking lot system
class ParkingLot {
private:
    std::vector<std::unique_ptr<ParkingLotLevel>> levels;
    std::unordered_map<int, int> spotToLevelMap; // maps spot ID to level number
    std::mutex mtx;
    std::unordered_map<std::string, Ticket> activeTickets;
    PaymentProcessor paymentProcessor;

public:
    // Constructor to initialize levels and the payment processor
    ParkingLot(int numLevels, double hourlyRate) : paymentProcessor(hourlyRate) {
        for (int i = 0; i < numLevels; ++i) {
            levels.push_back(std::make_unique<ParkingLotLevel>(i));
        }
    }

    // Adds a batch of spots to a specific level
    void addSpots(int level, SpotSize size, int count) {
        if (level < levels.size()) {
            for (int i = 0; i < count; ++i) {
                int spotId = static_cast<int>(levels[level]->getAvailableSpots(size));
                levels[level]->addSpot(std::make_unique<ParkingSpot>(spotId, size));
                // We're not using a global spot ID, so this mapping is simple for this example
            }
        }
    }

    // Parks a vehicle and generates a ticket
    std::string parkVehicle(std::unique_ptr<Vehicle> vehicle, const std::string& vehicleId) {
        std::lock_guard<std::mutex> lock(mtx);
        for (auto& level : levels) {
            std::unique_ptr<ParkingSpot> parkedSpot = level->findAndPark(std::move(vehicle));
            if (parkedSpot) {
                int spotId = parkedSpot->getId();
                Ticket newTicket(spotId, vehicleId);
                activeTickets[vehicleId] = newTicket;
                std::cout << "Vehicle " << vehicleId << " parked successfully at spot ID " << spotId << " on level " << level->getLevelNumber() << "." << std::endl;
                return vehicleId;
            }
        }
        return ""; // Parking failed
    }

    // Unparks a vehicle, calculates the fee, and processes payment
    bool unparkAndPay(const std::string& vehicleId, PaymentMethod method) {
        std::lock_guard<std::mutex> lock(mtx);
        if (activeTickets.count(vehicleId)) {
            Ticket& ticket = activeTickets.at(vehicleId);
            double fee = paymentProcessor.calculateFee(ticket);

            if (paymentProcessor.processPayment(fee, method)) {
                if (spotToLevelMap.count(ticket.spotId)) {
                    int level = spotToLevelMap[ticket.spotId];
                    if (levels[level]->unpark(ticket.spotId)) {
                        activeTickets.erase(vehicleId);
                        return true;
                    }
                }
            }
        }
        return false;
    }

    // Reports real-time availability of all spots
    std::map<SpotSize, int> getAvailability() const {
        std::map<SpotSize, int> availableSpots;
        availableSpots[SpotSize::Motorcycle] = 0;
        availableSpots[SpotSize::Compact] = 0;
        availableSpots[SpotSize::Large] = 0;
        for (const auto& level : levels) {
            availableSpots[SpotSize::Motorcycle] += level->getAvailableSpots(SpotSize::Motorcycle);
            availableSpots[SpotSize::Compact] += level->getAvailableSpots(SpotSize::Compact);
            availableSpots[SpotSize::Large] += level->getAvailableSpots(SpotSize::Large);
        }
        return availableSpots;
    }
};

