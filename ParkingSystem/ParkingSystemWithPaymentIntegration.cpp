#include "PaymentIntegration.h"


int main() {
    // Create a parking lot with 3 levels and a rate of $2.50 per hour
    ParkingLot myLot(3, 2.5);

    // Add spots to each level
    // Note: In a real-world scenario, spot IDs should be globally unique or managed more robustly.
    myLot.addSpots(0, SpotSize::Motorcycle, 5);
    myLot.addSpots(0, SpotSize::Compact, 10);
    myLot.addSpots(0, SpotSize::Large, 5);

    myLot.addSpots(1, SpotSize::Motorcycle, 3);
    myLot.addSpots(1, SpotSize::Compact, 15);
    myLot.addSpots(1, SpotSize::Large, 10);

    myLot.addSpots(2, SpotSize::Motorcycle, 7);
    myLot.addSpots(2, SpotSize::Compact, 20);
    myLot.addSpots(2, SpotSize::Large, 15);

    // Get initial availability
    auto availability = myLot.getAvailability();
    std::cout << "Initial Availability:" << std::endl;
    std::cout << "Motorcycle spots: " << availability[SpotSize::Motorcycle] << std::endl;
    std::cout << "Car spots: " << availability[SpotSize::Compact] << std::endl;
    std::cout << "Truck spots: " << availability[SpotSize::Large] << std::endl;

    // Park a car
    std::string myCarId = "XYZ-789";
    std::unique_ptr<Car> myCar = std::make_unique<Car>();
    std::string ticketId = myLot.parkVehicle(std::move(myCar), myCarId);

    if (!ticketId.empty()) {
        std::cout << "\nCar with ID " << ticketId << " is parked. Now simulating 2 hours passing..." << std::endl;

        // Simulate some time passing to accrue a fee
        std::this_thread::sleep_for(std::chrono::seconds(2)); // Use a short sleep for demonstration
        
        // Unpark the car and pay using a credit card
        std::cout << "\nAttempting to unpark vehicle " << myCarId << " and pay with Credit Card." << std::endl;
        bool unparked = myLot.unparkAndPay(myCarId, PaymentMethod::CreditCard);

        if (unparked) {
            std::cout << "Vehicle " << myCarId << " unparked and payment successful." << std::endl;
        } else {
            std::cout << "Unparking failed or vehicle not found." << std::endl;
        }
    } else {
        std::cout << "Parking failed, no spot available for the car." << std::endl;
    }

    return 0;
}
