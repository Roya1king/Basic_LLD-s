#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <mutex>
#include <memory>
#include <unordered_map>

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




class ParkingSpot {
private:
    int id;
    SpotSize size;
    bool isOccupied;
    std::unique_ptr<Vehicle> parkedVehicle;

public:
    ParkingSpot(int _id, SpotSize _size)
        : id(_id), size(_size), isOccupied(false) {}

    bool parkVehicle(std::unique_ptr<Vehicle> vehicle) {
        if (!isOccupied && vehicle->getSize() <= size) {
            parkedVehicle = std::move(vehicle);
            isOccupied = true;
            return true;
        }
        return false;
    }

    void unparkVehicle() {
        parkedVehicle.reset();
        isOccupied = false;
    }

    bool isAvailable() const { return !isOccupied; }
    int getId() const { return id; }
    SpotSize getSize() const { return size; }
};



class ParkingLotLevel {
private:
    int levelNumber;
    std::map<SpotSize, std::vector<std::unique_ptr<ParkingSpot>>> spots;
    std::mutex mtx;

public:
    ParkingLotLevel(int number) : levelNumber(number) {}

    void addSpot(std::unique_ptr<ParkingSpot> spot) {
        spots[spot->getSize()].push_back(std::move(spot));
    }

    std::unique_ptr<ParkingSpot> findAndPark(std::unique_ptr<Vehicle> vehicle) {
        std::lock_guard<std::mutex> lock(mtx);
        SpotSize vehicleSize = vehicle->getSize();

        for (auto& spot : spots[vehicleSize]) {
            if (spot->isAvailable()) {
                spot->parkVehicle(std::move(vehicle));
                return std::move(spot);
            }
        }
        return nullptr; // No spot available
    }

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
};



class ParkingLot {
private:
    std::vector<std::unique_ptr<ParkingLotLevel>> levels;
    std::unordered_map<int, int> spotToLevelMap; // maps spot ID to level number
    std::mutex mtx;

public:
    ParkingLot(int numLevels) {
        for (int i = 0; i < numLevels; ++i) {
            levels.push_back(std::make_unique<ParkingLotLevel>(i));
        }
    }

    void addSpots(int level, SpotSize size, int count) {
        if (level < levels.size()) {
            for (int i = 0; i < count; ++i) {
                levels[level]->addSpot(std::make_unique<ParkingSpot>(i, size));
                spotToLevelMap[i] = level;
            }
        }
    }

    int parkVehicle(std::unique_ptr<Vehicle> vehicle) {
        std::lock_guard<std::mutex> lock(mtx);
        for (auto& level : levels) {
            std::unique_ptr<ParkingSpot> parkedSpot = level->findAndPark(std::move(vehicle));
            if (parkedSpot) {
                return parkedSpot->getId();
            }
        }
        return -1; // No available spot
    }

    bool unparkVehicle(int spotId) {
        std::lock_guard<std::mutex> lock(mtx);
        if (spotToLevelMap.count(spotId)) {
            int level = spotToLevelMap[spotId];
            return levels[level]->unpark(spotId);
        }
        return false;
    }

    std::map<SpotSize, int> getAvailability() const {
        std::map<SpotSize, int> availableSpots;
        for (const auto& level : levels) {
            availableSpots[SpotSize::Motorcycle] += level->getAvailableSpots(SpotSize::Motorcycle);
            availableSpots[SpotSize::Compact] += level->getAvailableSpots(SpotSize::Compact);
            availableSpots[SpotSize::Large] += level->getAvailableSpots(SpotSize::Large);
        }
        return availableSpots;
    }
};