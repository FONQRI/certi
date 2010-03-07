// ----------------------------------------------------------------------------
// CERTI - HLA RunTime Infrastructure
// Copyright (C) 2002-2005  ONERA
//
// This program is free software ; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation ; either version 2 of
// the License, or (at your option) Any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY ; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program ; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
//
// $Id: NetworkMessage.hh,v 3.57 2010/03/07 18:23:40 erk Exp $
// ----------------------------------------------------------------------------

#ifndef CERTI_NETWORK_MESSAGE_HH
#define CERTI_NETWORK_MESSAGE_HH

#include "FedTimeD.hh"
#include "Exception.hh"
#include "Socket.hh"
#include "RTIRegion.hh"
#include "BasicMessage.hh"

#include <vector>
#include <string>

#ifdef FEDERATION_USES_MULTICAST
#define MC_PORT 60123
#define ADRESSE_MULTICAST "224.0.0.251"
#endif // FEDERATION_USES_MULTICAST

#define PORT_TCP_RTIG "60400"
#define PORT_UDP_RTIG "60500"

namespace certi {

/**
 * NetworkMessage is the base class used
 * for modelling message exchanged between RTIG and RTIA.
 * NetworkMessage is the base class of a class hierarchy.
 * Each specific message is a (direct of indirect)
 * daughter class of NetworkMessage.    
 */ 
class CERTI_EXPORT NetworkMessage : public BasicMessage
{
public:

	typedef enum Type {
		NOT_USED = 0, // Not used.
				CLOSE_CONNEXION,
				MESSAGE_NULL,
				CREATE_FEDERATION_EXECUTION,
				DESTROY_FEDERATION_EXECUTION,
				JOIN_FEDERATION_EXECUTION,
				RESIGN_FEDERATION_EXECUTION,
				SET_TIME_REGULATING,
				SET_TIME_CONSTRAINED,
				TIME_REGULATION_ENABLED, // RTIG to RTIA
				TIME_CONSTRAINED_ENABLED, // RTIG to RTIA
				REGISTER_FEDERATION_SYNCHRONIZATION_POINT,
				SYNCHRONIZATION_POINT_REGISTRATION_SUCCEEDED, // RTIG to RTIA
				ANNOUNCE_SYNCHRONIZATION_POINT, // RTIG to RTIA
				SYNCHRONIZATION_POINT_ACHIEVED,
				FEDERATION_SYNCHRONIZED, // RTIG to RTIA
				REQUEST_FEDERATION_SAVE,
				FEDERATE_SAVE_BEGUN,
				FEDERATE_SAVE_COMPLETE,
				FEDERATE_SAVE_NOT_COMPLETE,
				INITIATE_FEDERATE_SAVE, // RTIG to RTIA
				FEDERATION_SAVED, // RTIG to RTIA
				FEDERATION_NOT_SAVED, // RTIG to RTIA
				REQUEST_FEDERATION_RESTORE,
				FEDERATE_RESTORE_COMPLETE,
				FEDERATE_RESTORE_NOT_COMPLETE,
				REQUEST_FEDERATION_RESTORE_SUCCEEDED, // RTIG to RTIA
				REQUEST_FEDERATION_RESTORE_FAILED, // RTIG to RTIA
				FEDERATION_RESTORE_BEGUN, // RTIG to RTIA
				INITIATE_FEDERATE_RESTORE, // RTIG to RTIA
				FEDERATION_RESTORED, // RTIG to RTIA
				FEDERATION_NOT_RESTORED, // RTIG to RTIA
				PUBLISH_OBJECT_CLASS,
				UNPUBLISH_OBJECT_CLASS,
				PUBLISH_INTERACTION_CLASS,
				UNPUBLISH_INTERACTION_CLASS,
				SUBSCRIBE_OBJECT_CLASS,
				UNSUBSCRIBE_OBJECT_CLASS,
				SUBSCRIBE_INTERACTION_CLASS,
				UNSUBSCRIBE_INTERACTION_CLASS,
				TURN_INTERACTIONS_ON, // only RTIG->RTIA
				TURN_INTERACTIONS_OFF, // only RTIG->RTIA
				REGISTER_OBJECT,
				DISCOVER_OBJECT, // only RTIG->RTIA
				UPDATE_ATTRIBUTE_VALUES,
				REFLECT_ATTRIBUTE_VALUES, // only RTIG->RTIA
				SEND_INTERACTION,
				RECEIVE_INTERACTION, // only RTIG->RTIA
				DELETE_OBJECT,
				REMOVE_OBJECT, // only RTIG->RTIA
				CHANGE_ATTRIBUTE_TRANSPORT_TYPE,
				CHANGE_ATTRIBUTE_ORDER_TYPE,
				CHANGE_INTERACTION_TRANSPORT_TYPE,
				CHANGE_INTERACTION_ORDER_TYPE,
				REQUEST_CLASS_ATTRIBUTE_VALUE_UPDATE,
				REQUEST_OBJECT_ATTRIBUTE_VALUE_UPDATE,
				IS_ATTRIBUTE_OWNED_BY_FEDERATE,
				QUERY_ATTRIBUTE_OWNERSHIP,
				ATTRIBUTE_IS_NOT_OWNED,
				INFORM_ATTRIBUTE_OWNERSHIP,
				NEGOTIATED_ATTRIBUTE_OWNERSHIP_DIVESTITURE,
				ATTRIBUTE_OWNERSHIP_ACQUISITION_NOTIFICATION,
				ATTRIBUTE_OWNERSHIP_DIVESTITURE_NOTIFICATION,
				REQUEST_ATTRIBUTE_OWNERSHIP_ASSUMPTION,
				ATTRIBUTE_OWNERSHIP_UNAVAILABLE,
				ATTRIBUTE_OWNERSHIP_ACQUISITION_IF_AVAILABLE,
				UNCONDITIONAL_ATTRIBUTE_OWNERSHIP_DIVESTITURE,
				ATTRIBUTE_OWNERSHIP_ACQUISITION,
				REQUEST_ATTRIBUTE_OWNERSHIP_RELEASE,
				CANCEL_NEGOTIATED_ATTRIBUTE_OWNERSHIP_DIVESTITURE,
				ATTRIBUTE_OWNERSHIP_RELEASE_RESPONSE,
				CANCEL_ATTRIBUTE_OWNERSHIP_ACQUISITION,
				CONFIRM_ATTRIBUTE_OWNERSHIP_ACQUISITION_CANCELLATION,
				DDM_CREATE_REGION,
				DDM_MODIFY_REGION,
				DDM_DELETE_REGION,
				DDM_ASSOCIATE_REGION,
				DDM_REGISTER_OBJECT,
				DDM_UNASSOCIATE_REGION,
				DDM_SUBSCRIBE_ATTRIBUTES,
				DDM_UNSUBSCRIBE_ATTRIBUTES,
				DDM_SUBSCRIBE_INTERACTION,
				DDM_UNSUBSCRIBE_INTERACTION,
				PROVIDE_ATTRIBUTE_VALUE_UPDATE,
				GET_FED_FILE_SUPPRESSED,
				SET_CLASS_RELEVANCE_ADVISORY_SWITCH,
				SET_INTERACTION_RELEVANCE_ADVISORY_SWITCH,
				SET_ATTRIBUTE_RELEVANCE_ADVISORY_SWITCH,
				SET_ATTRIBUTE_SCOPE_ADVISORY_SWITCH,
				START_REGISTRATION_FOR_OBJECT_CLASS,
				STOP_REGISTRATION_FOR_OBJECT_CLASS,
				LAST
	} Message_T;	

	NetworkMessage();
	virtual ~NetworkMessage();

	const NetworkMessage::Message_T getType() const {return type;};
	const TypeException getException() const {return exception;};
	TypeException& getRefException() {return exception;};
	void setException(const TypeException except) {exception=except;};

	/**
	 * Serialize the message into a buffer
	 * @param[in] msgBuffer the serialization buffer
	 */
	virtual void serialize(MessageBuffer& msgBuffer);

	/**
	 * DeSerialize the message from a buffer
	 * @param[in] msgBuffer the deserialization buffer
	 */
	virtual void deserialize(MessageBuffer& msgBuffer);

	/**
	 * Send a message buffer to the socket
	 */
	void send(Socket* socket, MessageBuffer& msgBuffer) throw (NetworkError, NetworkSignal);
	void receive(Socket* socket, MessageBuffer& msgBuffer) throw (NetworkError, NetworkSignal);

	// Parameter and Attribute Management
	// Remove the Parameter of rank 'Rank' in the ParamArray and its value in
	// ValueArray. If there are other parameter in the list, they are shifted
	// one rank back.
	// Ex: ParamArraySize = 3,
	// ParamArray =[1, 2, 3], ValueArray =["Val1", "Val2", "Val3"]
	//->removeParameter(1)
	// ParamArraySize = 2,
	// ParamArray =[1, 3], ValueArray =["Val1", "Val3"]
	void removeParameter(UShort Rank);

	// See RemoveParameter for explanations.
	void removeAttribute(UShort Rank);

	// Value Array Management

	void setAHS(const std::vector <AttributeHandle> &, int);

	void displayValueArray(char *);

	int bestEffortPeer ;
	unsigned long bestEffortAddress ;

	int numberOfRegulators ;

	/* NM_DDM_Base class fields */
	SpaceHandle            space;
	int32_t                nbExtents;
	int32_t                region;
	ObjectHandle           object;
	ObjectClassHandle      objectClass;
	InteractionClassHandle interactionClass;

	unsigned long multicastAddress ;

	void sizeValueArray(int size) ;

	ObjectHandle firstId ;
	ObjectHandle lastId ;

	EventRetractionHandle eventRetraction ;

	/* NM_WithHandleArray class specific fields */
	uint16_t handleArraySize ;
	std::vector <AttributeHandle> handleArray ;
	std::vector <AttributeValue_t> valueArray ;

	TransportType transport ;
	OrderType order ;

	/** The name corresponding to message type */
	const char* getName() const {return name;}

	/**
	 * The federation handle 
	 * the message is part of this federation activity
	 */
	Handle federation ;
	/** 
	 * The federate handle
	 * the message is for this particular federate
	 */
	FederateHandle federate ;

	/**
	 * The exception reason (if the message carry one)
	 */
	std::string exceptionReason;

protected:

	/** 
	 * The message name.
	 * It should be initialized by the specialized
	 * network message constructor.
	 * From a performance point of view it's better to keep
	 * this as a plain char* because it won't be changed
	 * it's thus cheaper to build than an std::string.
	 */
	const char* name;	

	/** 
	 * The network message type
	 * type field cannot be accessed directly 
	 *   - only NM constructor may set it.
	 *   - getter should be used to get it. 
	 */
	Message_T type;

	/**
	 * The exception type
	 * if the message is carrying an exception
	 */
	TypeException exception ;

private:

};

// BUG: FIXME this is used by SocketMC and should
//      be thrown away as soon as possible.
#define TAILLE_MSG_RESEAU sizeof(NetworkMessage)

} // namespace certi

#endif // CERTI_NETWORK_MESSAGE_HH

// $Id: NetworkMessage.hh,v 3.57 2010/03/07 18:23:40 erk Exp $
