#include "Aircraft.h"

#ifndef SRC_COMMUNICATIONSYSTEM_H_
#define SRC_COMMUNICATIONSYSTEM_H_

using namespace std;

class CommunicationSystem {
public:
	// Constructor to initialize the CommunicationSystem object
	CommunicationSystem();

	/**
	 * Relays new speed values to a specific Aircraft.
	 *
	 * @param R The Aircraft object to receive the new speed values.
	 * @param newSpeedX The new speed value in the x-axis.
	 * @param newSpeedY The new speed value in the y-axis.
	 * @param newSpeedZ The new speed value in the z-axis.
	 * @return A void pointer indicating success or failure of the communication attempt.
	 */
	void* relayNewSpeed(Aircraft &R, int newSpeedX, int newSpeedY, int newSpeedZ);

	/**
	 * Relays new position coordinates to a specific Aircraft.
	 *
	 * @param R The Aircraft object to receive the new position values.
	 * @param newPositionX The new position value in the x-axis.
	 * @param newPositionY The new position value in the y-axis.
	 * @param newPositionZ The new position value in the z-axis.
	 * @return A void pointer indicating success or failure of the communication attempt.
	 */
	void* relayNewPosition(Aircraft &R, int newPositionX, int newPositionY, int newPositionZ);

	// Destructor to clean up any resources used by the CommunicationSystem object
	virtual ~CommunicationSystem();
};

#endif /* SRC_COMMUNICATIONSYSTEM_H_ */
