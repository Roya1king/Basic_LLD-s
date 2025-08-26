#include "ParkingSystem.h"

int main() {
    ParkingLot myLot(3); // A parking lot with 3 levels

    // Add spots to level 0
    myLot.addSpots(0, SpotSize::Motorcycle, 5);
    myLot.addSpots(0, SpotSize::Compact, 10);

    // Add spots to level 1
    myLot.addSpots(1, SpotSize::Compact, 15);
    myLot.addSpots(1, SpotSize::Large, 5);

    // Add spots to level 2
    myLot.addSpots(2, SpotSize::Motorcycle, 3);
    myLot.addSpots(2, SpotSize::Compact, 20);

    // Get initial availability
    auto availability = myLot.getAvailability();
    std::cout << "Initial Availability:" << std::endl;
    std::cout << "Motorcycle spots: " << availability[SpotSize::Motorcycle] << std::endl;
    std::cout << "Car spots: " << availability[SpotSize::Compact] << std::endl;
    std::cout << "Truck spots: " << availability[SpotSize::Large] << std::endl;

    // Park a car
    std::unique_ptr<Car> car = std::make_unique<Car>();
    int carSpotId = myLot.parkVehicle(std::move(car));
    if (carSpotId != -1) {
        std::cout << "Parked a car at spot ID: " << carSpotId << std::endl;
    } else {
        std::cout << "Parking lot is full for cars." << std::endl;
    }

    // Get updated availability
    availability = myLot.getAvailability();
    std::cout << "\nUpdated Availability:" << std::endl;
    std::cout << "Motorcycle spots: " << availability[SpotSize::Motorcycle] << std::endl;
    std::cout << "Car spots: " << availability[SpotSize::Compact] << std::endl;
    std::cout << "Truck spots: " << availability[SpotSize::Large] << std::endl;

    // Unpark the car
    if (myLot.unparkVehicle(carSpotId)) {
        std::cout << "\nUnparked the car from spot ID: " << carSpotId << std::endl;
    }

    return 0;
}