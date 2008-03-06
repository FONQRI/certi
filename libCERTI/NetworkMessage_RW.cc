// ----------------------------------------------------------------------------
// CERTI - HLA RunTime Infrastructure
// Copyright (C) 2002-2005  ONERA
//
// This program is free software ; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation ; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY ; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program ; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// $Id: NetworkMessage_RW.cc,v 3.45 2008/03/06 16:33:10 rousse Exp $
// ----------------------------------------------------------------------------



#include "NetworkMessage.hh"
#include "PrettyDebug.hh"

using std::vector ;
using std::endl;
using std::cout;

namespace certi {

static pdCDebug D("RTIG_MSG","(NetWorkMS) - ");
static PrettyDebug G("GENDOC",__FILE__);

// ----------------------------------------------------------------------------
void NetworkMessage::trace(const char* context)
{
#ifndef NDEBUG
D.Mes(pdMessage,'N',this->type,context);
#endif
}

// ----------------------------------------------------------------------------
// readBody
void
NetworkMessage::readBody(Socket *socket)
{
    MessageBody body ;
    unsigned short i ;
    G.Out(pdGendoc,"enter NetworkMessage::readBody");
    if (Header.bodySize == 0)
        throw RTIinternalError("ReadBody should not have been called.");

    // 1. Read Body from socket.
    socket->receive((void *) body.getBuffer(), Header.bodySize);
    // FIXME EN: we must update the write pointer of the 
	 //           MessageBody because we have just written 
	 //           on it using direct pointer access !! (nasty usage)
	 body.addToWritePointer(Header.bodySize);
	 
    // 3. Read informations from Message Body according to message type.
	//D.Mes(pdMessage, 'N',Header.type);
	this->trace("NetworkMessage::readBody ");

    if (Header.exception != e_NO_EXCEPTION) {
        body.readString(exceptionReason, MAX_EXCEPTION_REASON_LENGTH);
    }
    else {
    switch(Header.type) {
      // line number, FEDid, Value Array size and Value array (line contents)
      case GET_FED_FILE:
        number = body.readShortInt();
        readFEDid(body);
        if ( number >= 1 )  // open (0) and close (0) no more information
            {
            ValueArray[0].length = body.readLongInt();
            body.readBlock(ValueArray[0].value, ValueArray[0].length) ;
	    }
	break ;
            
      case UPDATE_ATTRIBUTE_VALUES:
	object = body.readLongInt();
	readLabel(body);
	boolean = body.readLongInt();   // true means with time
	body.readBlock((char *) handleArray, handleArraySize * sizeof(AttributeHandle));
	for (i = 0 ; i < handleArraySize ; i ++) {
            ValueArray[i].length = body.readLongInt();
            body.readBlock(ValueArray[i].value, ValueArray[i].length) ;
	}
	break ;

      case REFLECT_ATTRIBUTE_VALUES:
	object = body.readLongInt();
	readLabel(body);
	boolean = body.readLongInt();    // true means with time
	body.readBlock((char *) handleArray, handleArraySize * sizeof(AttributeHandle));
	for (i = 0 ; i < handleArraySize ; i ++) {
            ValueArray[i].length = body.readLongInt();
            body.readBlock(ValueArray[i].value, ValueArray[i].length) ;
	}
	break ;

      case PROVIDE_ATTRIBUTE_VALUE_UPDATE:
	object = body.readLongInt();
	for (i = 0 ; i < handleArraySize ; i ++)
	    handleArray[i] = body.readShortInt();
	break ;

	
	// -- O_I Variable Part With Date(Body Not Empty) --
      case SEND_INTERACTION:
      case RECEIVE_INTERACTION:
	readLabel(body);
	boolean = body.readLongInt();   // true means with time
	body.readBlock((char *) handleArray,
		       handleArraySize * sizeof(AttributeHandle));
	for (i = 0 ; i < handleArraySize ; i ++) {
            ValueArray[i].length = body.readLongInt() ;
            body.readBlock(ValueArray[i].value, ValueArray[i].length) ;
	}
	region = body.readLongInt();
	break ;


      case REQUEST_OBJECT_ATTRIBUTE_VALUE_UPDATE:
	object = body.readLongInt();
	for (i = 0 ; i < handleArraySize ; i ++)
	    handleArray[i] = body.readShortInt();
	break ;
        	
      case CREATE_FEDERATION_EXECUTION:
	readFederationName(body);
	readFEDid(body);
	break ;

      case DESTROY_FEDERATION_EXECUTION:
	readFederationName(body);
	break ;
	
      case REGISTER_FEDERATION_SYNCHRONIZATION_POINT:
	readLabel(body);
	readTag(body);
	boolean = body.readLongInt();
        // boolean true means there is an handleArray
        if ( boolean)
            {
            handleArraySize = body.readShortInt();
	    for (i = 0 ; i < handleArraySize ; i ++)
	        handleArray[i] = body.readShortInt();
            }
	break ;

      case ANNOUNCE_SYNCHRONIZATION_POINT:
	readLabel(body);
	readTag(body);
	break ;
	
      case SYNCHRONIZATION_POINT_ACHIEVED:
      case SYNCHRONIZATION_POINT_REGISTRATION_SUCCEEDED:
      case FEDERATION_SYNCHRONIZED:
      case REQUEST_FEDERATION_RESTORE:
	readLabel(body);
	break ;

      case REQUEST_FEDERATION_RESTORE_SUCCEEDED:
	readLabel(body);
	break ;

      case INITIATE_FEDERATE_RESTORE:
	readLabel(body);
	break ;

      case INITIATE_FEDERATE_SAVE:
	readLabel(body);
        // boolean true means with time (in the header)
	boolean = body.readLongInt();
	break ;

      case REQUEST_FEDERATION_SAVE:
	readLabel(body);
        // boolean true means with time (in the header)
        boolean = body.readLongInt();
	break ;
	
      case REQUEST_FEDERATION_RESTORE_FAILED:
	readLabel(body);
	readTag(body);
	break ;

      case DELETE_OBJECT:
      case REMOVE_OBJECT:
	object = body.readLongInt();
	boolean = body.readLongInt();   // true means with time
	readLabel(body);
	break ;

	// -- No Variable Part --

      case IS_ATTRIBUTE_OWNED_BY_FEDERATE:
      case INFORM_ATTRIBUTE_OWNERSHIP:
      case ATTRIBUTE_IS_NOT_OWNED:
      case QUERY_ATTRIBUTE_OWNERSHIP:
	object = body.readLongInt();
	handleArray[0] = body.readShortInt();
	readLabel(body);
	break ;

      case NEGOTIATED_ATTRIBUTE_OWNERSHIP_DIVESTITURE:
      case REQUEST_ATTRIBUTE_OWNERSHIP_ASSUMPTION:
      case ATTRIBUTE_OWNERSHIP_ACQUISITION:
      case REQUEST_ATTRIBUTE_OWNERSHIP_RELEASE:
	object = body.readLongInt();
	handleArraySize = body.readShortInt();
	for (i = 0 ; i < handleArraySize ; i ++)
	    handleArray[i] = body.readShortInt();
	readLabel(body);
	break ;

      case ATTRIBUTE_OWNERSHIP_ACQUISITION_IF_AVAILABLE:
      case ATTRIBUTE_OWNERSHIP_ACQUISITION_NOTIFICATION:
      case ATTRIBUTE_OWNERSHIP_UNAVAILABLE:
      case UNCONDITIONAL_ATTRIBUTE_OWNERSHIP_DIVESTITURE:
      case ATTRIBUTE_OWNERSHIP_DIVESTITURE_NOTIFICATION:
      case CANCEL_NEGOTIATED_ATTRIBUTE_OWNERSHIP_DIVESTITURE:
      case ATTRIBUTE_OWNERSHIP_RELEASE_RESPONSE:
      case CANCEL_ATTRIBUTE_OWNERSHIP_ACQUISITION:
      case CONFIRM_ATTRIBUTE_OWNERSHIP_ACQUISITION_CANCELLATION:
	object = body.readLongInt();
	handleArraySize = body.readShortInt();
	for (i = 0 ; i < handleArraySize ; i ++)
	    handleArray[i] = body.readShortInt();
	break ;

      case DDM_CREATE_REGION:
	space = body.readLongInt();
	nbExtents = body.readLongInt();
	region = body.readLongInt();
	break ;

	// -- Join Variable Part --

      case JOIN_FEDERATION_EXECUTION:
	readFederationName(body);
	readFederateName(body);
	break ;

	// -- O_I Variable Part(Body not empty) --

      case PUBLISH_OBJECT_CLASS:
      case SUBSCRIBE_OBJECT_CLASS:
	for (i = 0 ; i < handleArraySize ; i ++)
	    handleArray[i] = body.readShortInt();
	break ;

      case REGISTER_OBJECT:
      case DISCOVER_OBJECT:
	object = body.readLongInt();
	readLabel(body);
	break ;

      case DDM_MODIFY_REGION:
	readExtents(body);
	break ;

      case DDM_ASSOCIATE_REGION:
	object = body.readLongInt();
	region = body.readLongInt();
	boolean = body.readLongInt();
	handleArraySize = body.readShortInt();
	for (i = 0 ; i < handleArraySize ; i ++)
	    handleArray[i] = body.readShortInt();
	break ;

      case DDM_SUBSCRIBE_ATTRIBUTES:
	objectClass = body.readLongInt();
	region = body.readLongInt();
	boolean = body.readLongInt();
	handleArraySize = body.readShortInt();
	for (i = 0 ; i < handleArraySize ; i ++)
	    handleArray[i] = body.readShortInt();
	break ;

      case DDM_UNASSOCIATE_REGION:
	object = body.readLongInt();
	region = body.readLongInt();
	break ;

      case DDM_UNSUBSCRIBE_ATTRIBUTES:	    
	objectClass = body.readLongInt();
	region = body.readLongInt();
	break ;

      case DDM_SUBSCRIBE_INTERACTION:
      case DDM_UNSUBSCRIBE_INTERACTION:
	interactionClass = body.readLongInt();
	region = body.readLongInt();
	boolean = body.readLongInt();
	break ;

      case DDM_REGISTER_OBJECT:
	objectClass = body.readLongInt();
	object = body.readLongInt();
	region = body.readLongInt();
	readTag(body);
	handleArraySize = body.readShortInt();
	for (i = 0 ; i < handleArraySize ; i ++)
	    handleArray[i] = body.readShortInt();
	break ;
	    
      default:
	D.Out(pdExcept, "Unknown type %d in ReadBody.", Header.type);
	throw RTIinternalError("Unknown/Unimplemented type for body.");
    }
    }
    G.Out(pdGendoc,"exit NetworkMessage::readBody");
}

// ----------------------------------------------------------------------------
bool
NetworkMessage::readHeader(Socket *socket)
{
    G.Out(pdGendoc,"enter NetworkMessage::readHeader");
    // 1- Read Header from Socket
    socket->receive((void *) &Header, sizeof(HeaderStruct));
    // 2- Parse Header(Static Part)
    type = Header.type ;
    exception = Header.exception ;
    federate = Header.federate ;
    federation = Header.federation ;
    // If the message carry an exception, the Body will only contain the
    // exception reason.

    if (exception != e_NO_EXCEPTION)
        {
        G.Out(pdGendoc,"exit  Message::readHeader carrying an exception");
        return true ;
        }

    // 2- Parse Header according to its type(Variable Part)
    switch (Header.type) {
      case MESSAGE_NULL:
	date = Header.VP.time.date ;
	break ;

      case REQUEST_FEDERATION_SAVE:
	date = Header.VP.O_I.date ;
	break ;

      case UPDATE_ATTRIBUTE_VALUES:
	objectClass = Header.VP.O_I.handle ;
	handleArraySize = Header.VP.O_I.size ;
	date = Header.VP.O_I.date ;
	break ;

      case REFLECT_ATTRIBUTE_VALUES:
	objectClass = Header.VP.O_I.handle ;
	handleArraySize = Header.VP.O_I.size ;
	date = Header.VP.O_I.date ;
	break ;

      case PROVIDE_ATTRIBUTE_VALUE_UPDATE:
	objectClass = Header.VP.O_I.handle ;
	handleArraySize = Header.VP.O_I.size ;
	date = Header.VP.O_I.date ;
	break ;

      case SEND_INTERACTION:
      case RECEIVE_INTERACTION:
	interactionClass = Header.VP.O_I.handle ;
	handleArraySize = Header.VP.O_I.size ;
	date = Header.VP.O_I.date ;
	break ;

      case INITIATE_FEDERATE_SAVE:
	date = Header.VP.O_I.date ;
	break ;

      case REQUEST_OBJECT_ATTRIBUTE_VALUE_UPDATE:
 	handleArraySize = Header.VP.O_I.size ;
        break;
       
      case REQUEST_FEDERATION_RESTORE:
      case INITIATE_FEDERATE_RESTORE:
      case REQUEST_FEDERATION_RESTORE_SUCCEEDED:
      case REQUEST_FEDERATION_RESTORE_FAILED:
      case CREATE_FEDERATION_EXECUTION:
      case DESTROY_FEDERATION_EXECUTION:
      case REGISTER_FEDERATION_SYNCHRONIZATION_POINT:
      case SYNCHRONIZATION_POINT_ACHIEVED:
      case SYNCHRONIZATION_POINT_REGISTRATION_SUCCEEDED:
      case FEDERATION_SYNCHRONIZED:
      case ANNOUNCE_SYNCHRONIZATION_POINT:
      case DELETE_OBJECT:
      case REMOVE_OBJECT:
        date = Header.VP.O_I.date;
	break;
      case CLOSE_CONNEXION:
      case RESIGN_FEDERATION_EXECUTION:
      case IS_ATTRIBUTE_OWNED_BY_FEDERATE:
      case INFORM_ATTRIBUTE_OWNERSHIP:
      case ATTRIBUTE_IS_NOT_OWNED:
      case QUERY_ATTRIBUTE_OWNERSHIP:
      case ATTRIBUTE_OWNERSHIP_ACQUISITION_IF_AVAILABLE:
      case ATTRIBUTE_OWNERSHIP_ACQUISITION_NOTIFICATION:
      case ATTRIBUTE_OWNERSHIP_UNAVAILABLE:
      case NEGOTIATED_ATTRIBUTE_OWNERSHIP_DIVESTITURE:
      case REQUEST_ATTRIBUTE_OWNERSHIP_ASSUMPTION:
      case UNCONDITIONAL_ATTRIBUTE_OWNERSHIP_DIVESTITURE:
      case ATTRIBUTE_OWNERSHIP_ACQUISITION:
      case REQUEST_ATTRIBUTE_OWNERSHIP_RELEASE:
      case ATTRIBUTE_OWNERSHIP_DIVESTITURE_NOTIFICATION:
      case CANCEL_NEGOTIATED_ATTRIBUTE_OWNERSHIP_DIVESTITURE:
      case ATTRIBUTE_OWNERSHIP_RELEASE_RESPONSE:
      case CANCEL_ATTRIBUTE_OWNERSHIP_ACQUISITION:
      case CONFIRM_ATTRIBUTE_OWNERSHIP_ACQUISITION_CANCELLATION:
      case DDM_CREATE_REGION:
      case FEDERATE_SAVE_BEGUN:
      case FEDERATE_SAVE_COMPLETE:
      case FEDERATE_SAVE_NOT_COMPLETE:
      case FEDERATION_SAVED:
      case FEDERATION_NOT_SAVED:
      case FEDERATE_RESTORE_COMPLETE:
      case FEDERATE_RESTORE_NOT_COMPLETE:
      case FEDERATION_RESTORE_BEGUN:
      case FEDERATION_RESTORED:
      case FEDERATION_NOT_RESTORED:
      case DDM_ASSOCIATE_REGION:
      case DDM_UNASSOCIATE_REGION:
      case DDM_SUBSCRIBE_ATTRIBUTES:
      case DDM_UNSUBSCRIBE_ATTRIBUTES:
      case DDM_SUBSCRIBE_INTERACTION:
      case DDM_UNSUBSCRIBE_INTERACTION:
      case DDM_REGISTER_OBJECT:
      case GET_FED_FILE:
	break ;

      case SET_TIME_REGULATING:
	date = Header.VP.time.date ;
	regulator = Header.VP.time.R_or_C ;
	break ;

      case SET_TIME_CONSTRAINED:
	date = Header.VP.time.date ;
	constrained = Header.VP.time.R_or_C ;
	break ;

      case CHANGE_ATTRIBUTE_TRANSPORT_TYPE:
      case CHANGE_ATTRIBUTE_ORDER_TYPE:
      case CHANGE_INTERACTION_TRANSPORT_TYPE:
      case CHANGE_INTERACTION_ORDER_TYPE:
	throw RTIinternalError("Read Message not implemented for T/O.");
	break ;

	// -- Join Variable Part(No body) --

      case JOIN_FEDERATION_EXECUTION:
	numberOfRegulators = Header.VP.Join.NbReg ;
	multicastAddress = Header.VP.Join.AdrMC ;
	bestEffortAddress = Header.VP.Join.Addr ;
	bestEffortPeer = Header.VP.Join.peer ;
	break ;

	// -- O_I Variable Part(No body) --

      case UNPUBLISH_OBJECT_CLASS:
      case UNSUBSCRIBE_OBJECT_CLASS:
	objectClass = Header.VP.O_I.handle ;
	break ;

      case PUBLISH_INTERACTION_CLASS:
      case UNPUBLISH_INTERACTION_CLASS:
      case SUBSCRIBE_INTERACTION_CLASS:
      case UNSUBSCRIBE_INTERACTION_CLASS:
      case TURN_INTERACTIONS_ON:
      case TURN_INTERACTIONS_OFF:
	interactionClass = Header.VP.O_I.handle ;
	break ;

	// DDM variable part
      case DDM_DELETE_REGION:
	region = Header.VP.ddm.region ;
	break ;

	// -- O_I Variable Part(body not empty) --

      case PUBLISH_OBJECT_CLASS:
      case SUBSCRIBE_OBJECT_CLASS:
	objectClass = Header.VP.O_I.handle ;
	handleArraySize = Header.VP.O_I.size ;
	break ;

      case REGISTER_OBJECT:
      case DISCOVER_OBJECT:
	objectClass = Header.VP.O_I.handle ;
	break ;

      case DDM_MODIFY_REGION:
	region = Header.VP.ddm.region ;
	break ;

	// -- Default Handler --

      default:
	D.Out(pdExcept, "Unknown type %d in ReadHeader.", Header.type);
        G.Out(pdGendoc,"exit  NetworkMessage::readHeader with unknown type=%d",Header.type);
	throw RTIinternalError("Received unknown Header type.");
    }

    // 4- If Header.bodySize is not 0, return RTI_TRUE, else RTI_FALSE
    G.Out(pdGendoc,"exit  NetworkMessage::readHeader");
    return Header.bodySize ;
}

// ----------------------------------------------------------------------------
void
NetworkMessage::writeBody(Socket *socket)
{
    MessageBody body ;
    unsigned short i ;

    G.Out(pdGendoc,"enter NetworkMessage::writeBody");

    // 0- Copy the Header at the beginning of the body, in order to
    // make a single Socket->Emettre call while sending both.
    // WARNING: As the body size is not known yet, we will have to
    // change it in the copy also!
    body.writeBlock(reinterpret_cast<char *>(&Header), sizeof(HeaderStruct));
    
    D.Out(pdTrace, "HeaderStruct size is : <%d> out of <%d> bytes MAX in body\n",
	  sizeof(HeaderStruct),BUFFER_SIZE_DEFAULT);

   // If the message carry an exception, the Body will only contain the
   // exception reason.

   if (Header.exception != e_NO_EXCEPTION) {
        body.writeString(exceptionReason);
   }
   else
    {
    // 1- Prepare body Structure according to Message type
    switch(Header.type) {
      case GET_FED_FILE:
        body.writeShortInt(number);
        writeFEDid(body);
        if ( number >= 1 )  // open (0) and close (0) no more information
            {
            body.writeLongInt(ValueArray[0].length);
            body.writeBlock(ValueArray[0].value, ValueArray[0].length);
            }
      break;

      case UPDATE_ATTRIBUTE_VALUES:
	body.writeLongInt(object);
	body.writeString(label);
	body.writeLongInt(boolean);    // true means with time (stored in header)
	body.writeBlock((char *) handleArray, handleArraySize * sizeof(AttributeHandle));
	
	for (i = 0 ; i < handleArraySize ; i ++) {
            body.writeLongInt(ValueArray[i].length) ;
            body.writeBlock(ValueArray[i].value, ValueArray[i].length);
	}
	break ;

      case REFLECT_ATTRIBUTE_VALUES:
	body.writeLongInt(object);
	body.writeString(label);
        body.writeLongInt(boolean);
	body.writeBlock((char *) handleArray, handleArraySize * sizeof(AttributeHandle));	
	for (i = 0 ; i < handleArraySize ; i ++) {
            body.writeLongInt(ValueArray[i].length) ;
            body.writeBlock(ValueArray[i].value, ValueArray[i].length);
	}
	break ;

      case PROVIDE_ATTRIBUTE_VALUE_UPDATE:
	body.writeLongInt(object);
	for (i = 0 ; i < handleArraySize ; i ++)
            {
	    body.writeShortInt(handleArray[i]);
	    } 
	break ;
	
	// -- O_I Variable Part With date(body Not Empty) --
	    
      case SEND_INTERACTION:
      case RECEIVE_INTERACTION:
	body.writeString(label);
	body.writeLongInt(boolean);    // true means with time (stored in header)
	body.writeBlock((char *) handleArray,
			handleArraySize * sizeof(AttributeHandle));
	for (i = 0 ; i < handleArraySize ; i ++) {
            body.writeLongInt(ValueArray[i].length);
            body.writeBlock(ValueArray[i].value, ValueArray[i].length);
	}
        body.writeLongInt(region);
	break ;

      case REQUEST_OBJECT_ATTRIBUTE_VALUE_UPDATE:
	body.writeLongInt(object);	
	for (i = 0 ; i < handleArraySize ; i ++)
            {
	    body.writeShortInt(handleArray[i]);
	    } 
        break;
       
	// -- No Variable Part --

      case CREATE_FEDERATION_EXECUTION:
	writeFederationName(body);
        writeFEDid(body);
	break ;

      case DESTROY_FEDERATION_EXECUTION:
	writeFederationName(body);
	break ;

      case REGISTER_FEDERATION_SYNCHRONIZATION_POINT:
	body.writeString(label);
	body.writeString(tag);
	body.writeLongInt(boolean);
        // boolean true means we have an handleArray
        if ( boolean )
            {
	    body.writeShortInt(handleArraySize);
	    for (i = 0 ; i < handleArraySize ; i ++)
	        body.writeShortInt(handleArray[i]);
            }        
	break ;

      case ANNOUNCE_SYNCHRONIZATION_POINT:
	body.writeString(label);
	body.writeString(tag);       
	break ;

      case SYNCHRONIZATION_POINT_ACHIEVED:
      case SYNCHRONIZATION_POINT_REGISTRATION_SUCCEEDED:
      case FEDERATION_SYNCHRONIZED:
      case REQUEST_FEDERATION_RESTORE:
	body.writeString(label);
	break ;

      case REQUEST_FEDERATION_RESTORE_SUCCEEDED:
	body.writeString(label);
	break ;


      case INITIATE_FEDERATE_RESTORE:
	body.writeString(label);
	break ;

      case INITIATE_FEDERATE_SAVE:
	body.writeString(label);
        // boolean true means with time (in the header)
        body.writeLongInt(boolean);
	break ;

      case REQUEST_FEDERATION_SAVE:
        body.writeString(label);
        // boolean true means with time (in the header)
        body.writeLongInt(boolean);
	break ;

      case REQUEST_FEDERATION_RESTORE_FAILED:
	body.writeString(label);
	body.writeString(tag);
	break ;

      case DELETE_OBJECT:
      case REMOVE_OBJECT:
	body.writeLongInt(object);
	body.writeLongInt(boolean);    // true means with time (stored in header)
	body.writeString(label);
	break ;

      case IS_ATTRIBUTE_OWNED_BY_FEDERATE:
      case INFORM_ATTRIBUTE_OWNERSHIP:
      case ATTRIBUTE_IS_NOT_OWNED:
      case QUERY_ATTRIBUTE_OWNERSHIP:
	body.writeLongInt(object);
	body.writeShortInt(handleArray[0]);
	body.writeString(label);
	break ;

      case NEGOTIATED_ATTRIBUTE_OWNERSHIP_DIVESTITURE:
      case REQUEST_ATTRIBUTE_OWNERSHIP_ASSUMPTION:
      case ATTRIBUTE_OWNERSHIP_ACQUISITION:
      case REQUEST_ATTRIBUTE_OWNERSHIP_RELEASE:
	body.writeLongInt(object);
	body.writeShortInt(handleArraySize);
	for (i = 0 ; i < handleArraySize ; i ++)
	    body.writeShortInt(handleArray[i]);
	body.writeString(label);
	break ;


      case ATTRIBUTE_OWNERSHIP_ACQUISITION_IF_AVAILABLE:
      case ATTRIBUTE_OWNERSHIP_ACQUISITION_NOTIFICATION:
      case ATTRIBUTE_OWNERSHIP_UNAVAILABLE:
      case UNCONDITIONAL_ATTRIBUTE_OWNERSHIP_DIVESTITURE:
      case ATTRIBUTE_OWNERSHIP_DIVESTITURE_NOTIFICATION:
      case CANCEL_NEGOTIATED_ATTRIBUTE_OWNERSHIP_DIVESTITURE:
      case ATTRIBUTE_OWNERSHIP_RELEASE_RESPONSE:
      case CANCEL_ATTRIBUTE_OWNERSHIP_ACQUISITION:
      case CONFIRM_ATTRIBUTE_OWNERSHIP_ACQUISITION_CANCELLATION:
	body.writeLongInt(object);
	body.writeShortInt(handleArraySize);
	for (i = 0 ; i < handleArraySize ; i ++)
	    body.writeShortInt(handleArray[i]);
	break ;

      case DDM_CREATE_REGION:
	body.writeLongInt(space);
	body.writeLongInt(nbExtents);
	body.writeLongInt(region);
	break ;

	// -- Join Variable Part --

      case JOIN_FEDERATION_EXECUTION:
	writeFederationName(body);
	body.writeString(federateName);
	break ;

	// -- O_I Variable Part(body not empty) --

      case PUBLISH_OBJECT_CLASS:
      case SUBSCRIBE_OBJECT_CLASS:
	for (i = 0 ; i < handleArraySize ; i ++)
	    body.writeShortInt(handleArray[i]);
	break ;

      case REGISTER_OBJECT:
      case DISCOVER_OBJECT:
	body.writeLongInt(object);
	body.writeString(label);
	break ;

      case DDM_MODIFY_REGION:
	writeExtents(body);
	break ;

      case DDM_ASSOCIATE_REGION:
	body.writeLongInt(object);
	body.writeLongInt(region);
	body.writeLongInt(boolean);
	body.writeShortInt(handleArraySize);
	for (i = 0 ; i < handleArraySize ; i ++)
	    body.writeShortInt(handleArray[i]);
	break ;

      case DDM_SUBSCRIBE_ATTRIBUTES:
	body.writeLongInt(objectClass);
	body.writeLongInt(region);
	body.writeLongInt(boolean);
	body.writeShortInt(handleArraySize);
	for (i = 0 ; i < handleArraySize ; i ++)
	    body.writeShortInt(handleArray[i]);
	break ;

      case DDM_UNASSOCIATE_REGION:
	body.writeLongInt(object);
	body.writeLongInt(region);
	break ;

      case DDM_UNSUBSCRIBE_ATTRIBUTES:
	body.writeLongInt(objectClass);
	body.writeLongInt(region);
	break ;
	    
      case DDM_SUBSCRIBE_INTERACTION:
      case DDM_UNSUBSCRIBE_INTERACTION:
	body.writeLongInt(interactionClass);
	body.writeLongInt(region);
	body.writeLongInt(boolean);
	break ;

      case DDM_REGISTER_OBJECT:
	body.writeLongInt(objectClass);
	body.writeLongInt(object);
	body.writeLongInt(region);
	body.writeString(tag);
	body.writeShortInt(handleArraySize);
	for (i = 0 ; i < handleArraySize ; i ++)
	    body.writeShortInt(handleArray[i]);
	break ;
	    
	// -- Default Handler --
      default:
	D.Out(pdExcept, "Unknown type %d in Writebody.", Header.type);
	throw RTIinternalError("Unknown/Unimplemented type for Header.");
    }
 }

    // body Size does not include the copy of the Header!
    Header.bodySize = body.size() - sizeof(HeaderStruct);

    // Put the real body Size in the copy of the Header.
    // FIXME do we really need the body size in the header??
    (reinterpret_cast<HeaderStruct *>(body.getBufferModeRW()))->bodySize = Header.bodySize ;
    D.Out(pdTrace,"Sending MessageBody of size <%d>",body.size());
    socket->send(body.getBuffer(), body.size());

    G.Out(pdGendoc,"exit  NetworkMessage::writeBody");
}

// ----------------------------------------------------------------------------
bool
NetworkMessage::writeHeader(Socket *socket)
{
    G.Out(pdGendoc,"enter NetworkMessage::writeHeader");
    // 2- Fill Header(Static Part)
    Header.type = type ;
    Header.exception = exception ;
    Header.federate = federate ;
    Header.federation = federation ;
    // If the message carry an exception, the Body will only contain the
    // exception reason.

    if (exception != e_NO_EXCEPTION) {
        Header.bodySize = 1 ;
        G.Out(pdGendoc,"exit  NetworkMessage::writeHeader carrying an exception");
        return true ;
    }

    // 3- Fill Header(Variable Part)[Sorted by Variable part type]
    // Note: Header.bodySize is not set to the actual body size, but
    // to zero to indicate there is no body, or 1 if a body is needed.
    switch(type) {
      case MESSAGE_NULL:
	Header.bodySize = 0 ;
        Header.VP.time.date = date ;
	break ;
	
      case UPDATE_ATTRIBUTE_VALUES:
	Header.bodySize = 1 ;
        Header.VP.O_I.handle = objectClass ;
        Header.VP.O_I.size = handleArraySize ;
        Header.VP.O_I.date = date ;
	break ;

      case REFLECT_ATTRIBUTE_VALUES:
	Header.bodySize = 1 ;
        Header.VP.O_I.handle = objectClass ;
        Header.VP.O_I.size = handleArraySize ;
        Header.VP.O_I.date = date ;
	break ;

      case PROVIDE_ATTRIBUTE_VALUE_UPDATE:
	Header.bodySize = 1 ;
        Header.VP.O_I.handle = objectClass ;
        Header.VP.O_I.size = handleArraySize ;
	break ;
	
      case SEND_INTERACTION:
      case RECEIVE_INTERACTION:
	// body contains handleArray, ValueArray, label.
	Header.bodySize = 1 ;
	Header.VP.O_I.handle = interactionClass ;
	Header.VP.O_I.size = handleArraySize ;
	Header.VP.O_I.date = date ;
	break ;

      case REQUEST_FEDERATION_SAVE:
	Header.bodySize = 1 ;
        // boolean true means with time
        if ( boolean)
	    Header.VP.O_I.date = date ;
	break ;

      case INITIATE_FEDERATE_SAVE:
        // boolean true means with time
        if ( boolean)
	    Header.VP.O_I.date = date ;
	Header.bodySize = 1 ;
	break ;

      case REQUEST_FEDERATION_RESTORE:
      case REQUEST_FEDERATION_RESTORE_SUCCEEDED:
      case REQUEST_FEDERATION_RESTORE_FAILED:
	Header.bodySize = 1 ;
	break ;

      // Body contains Object handle,handleArray
      case REQUEST_OBJECT_ATTRIBUTE_VALUE_UPDATE:
        Header.bodySize = 1 ;
	Header.VP.O_I.size = handleArraySize ;
        break;

	// -- No Variable Part, No body --

      case CLOSE_CONNEXION:
      case RESIGN_FEDERATION_EXECUTION:
      case FEDERATE_SAVE_BEGUN:
      case FEDERATE_SAVE_COMPLETE:
      case FEDERATE_SAVE_NOT_COMPLETE:
      case FEDERATION_SAVED:
      case FEDERATION_NOT_SAVED:
      case FEDERATE_RESTORE_COMPLETE:
      case FEDERATE_RESTORE_NOT_COMPLETE:
      case FEDERATION_RESTORE_BEGUN:
      case FEDERATION_RESTORED:
      case FEDERATION_NOT_RESTORED:
	Header.bodySize = 0 ;
	break ;

	// -- No Variable Part, body not empty --

      case CREATE_FEDERATION_EXECUTION:
      case DESTROY_FEDERATION_EXECUTION:
	// body Contains federationName.
      case INFORM_ATTRIBUTE_OWNERSHIP:
      case ATTRIBUTE_IS_NOT_OWNED:
      case IS_ATTRIBUTE_OWNED_BY_FEDERATE:
      case QUERY_ATTRIBUTE_OWNERSHIP:
	// body Contains ObjectHandle and label
      case ATTRIBUTE_OWNERSHIP_ACQUISITION_IF_AVAILABLE:
      case ATTRIBUTE_OWNERSHIP_ACQUISITION_NOTIFICATION:
      case ATTRIBUTE_OWNERSHIP_UNAVAILABLE:
	// body Contains ObjectHandle and handleArray
      case NEGOTIATED_ATTRIBUTE_OWNERSHIP_DIVESTITURE:
      case REQUEST_ATTRIBUTE_OWNERSHIP_ASSUMPTION:
      case UNCONDITIONAL_ATTRIBUTE_OWNERSHIP_DIVESTITURE:
      case ATTRIBUTE_OWNERSHIP_ACQUISITION:
      case REQUEST_ATTRIBUTE_OWNERSHIP_RELEASE:
      case ATTRIBUTE_OWNERSHIP_DIVESTITURE_NOTIFICATION:
      case CANCEL_NEGOTIATED_ATTRIBUTE_OWNERSHIP_DIVESTITURE:
      case ATTRIBUTE_OWNERSHIP_RELEASE_RESPONSE:
      case CANCEL_ATTRIBUTE_OWNERSHIP_ACQUISITION:
      case CONFIRM_ATTRIBUTE_OWNERSHIP_ACQUISITION_CANCELLATION:
      case DDM_CREATE_REGION:
      case INITIATE_FEDERATE_RESTORE:
      case DDM_ASSOCIATE_REGION:
      case DDM_UNASSOCIATE_REGION:
      case DDM_SUBSCRIBE_ATTRIBUTES:
      case DDM_UNSUBSCRIBE_ATTRIBUTES:
      case DDM_SUBSCRIBE_INTERACTION:
      case DDM_UNSUBSCRIBE_INTERACTION:	
      case DDM_REGISTER_OBJECT:
      case GET_FED_FILE:
	Header.bodySize = 1 ;
	break ;

      case REGISTER_FEDERATION_SYNCHRONIZATION_POINT:
      case SYNCHRONIZATION_POINT_ACHIEVED:
      case SYNCHRONIZATION_POINT_REGISTRATION_SUCCEEDED:
      case FEDERATION_SYNCHRONIZED:
      case ANNOUNCE_SYNCHRONIZATION_POINT:
	// body Contains Label(should be non-empty)
	// BUG: S'il fait moins de 16 octet, il passe dans le header.
	Header.bodySize = 1 ;
	break ;

      case DELETE_OBJECT:
        Header.bodySize = 1;
	Header.VP.O_I.date = date;
	break;
      case REMOVE_OBJECT:
	// body Contains ObjectHandle, and label
	Header.bodySize = 1 ;
        if ( boolean)
	    Header.VP.O_I.date = date ;
	break ;

	// -- time Variable Part(No body)[Continued] --

      case SET_TIME_REGULATING:
	Header.bodySize = 0 ;
	Header.VP.time.date = date ;
	Header.VP.time.R_or_C = regulator ;
	break ;

      case SET_TIME_CONSTRAINED:
	Header.bodySize = 0 ;
	Header.VP.time.date = date ;
	Header.VP.time.R_or_C = constrained ;
	break ;

	// -- T_O Variable Part --

      case CHANGE_ATTRIBUTE_TRANSPORT_TYPE:
      case CHANGE_ATTRIBUTE_ORDER_TYPE:
      case CHANGE_INTERACTION_TRANSPORT_TYPE:
      case CHANGE_INTERACTION_ORDER_TYPE:
	throw RTIinternalError("Write Message not implemented for T/O.");
	break ;

	// -- Join Variable Part --

      case JOIN_FEDERATION_EXECUTION:
	// body contains federationName and federateName
	Header.bodySize = 1 ;
	Header.VP.Join.NbReg = numberOfRegulators ;
	Header.VP.Join.AdrMC = multicastAddress ;
	Header.VP.Join.Addr = bestEffortAddress ;
	Header.VP.Join.peer = bestEffortPeer ;
	break ;

	// -- O_I Variable Part(No body) --

      case UNPUBLISH_OBJECT_CLASS:
      case UNSUBSCRIBE_OBJECT_CLASS:
	Header.bodySize = 0 ;
	Header.VP.O_I.handle = objectClass ;
	break ;

      case PUBLISH_INTERACTION_CLASS:
      case UNPUBLISH_INTERACTION_CLASS:
      case SUBSCRIBE_INTERACTION_CLASS:
      case UNSUBSCRIBE_INTERACTION_CLASS:
      case TURN_INTERACTIONS_ON:
      case TURN_INTERACTIONS_OFF:
	Header.bodySize = 0 ;
	Header.VP.O_I.handle = interactionClass ;
	break ;

	// DDM variable part, no body
      case DDM_DELETE_REGION:
	Header.bodySize = 0 ;
	Header.VP.ddm.region = region ;
	break ;

	// -- O_I Variable Part(body not empty) --

      case PUBLISH_OBJECT_CLASS:
      case SUBSCRIBE_OBJECT_CLASS:
	// body contains handleArray[handleArraySize](if not empty)

	if (handleArraySize > 0)
	    Header.bodySize = 1 ;
	else
	    Header.bodySize = 0 ;

	Header.VP.O_I.handle = objectClass ;
	Header.VP.O_I.size = handleArraySize ;
	break ;

      case REGISTER_OBJECT:
      case DISCOVER_OBJECT:
	// body Contains ObjectHandle and label
	Header.bodySize = 1 ;
	Header.VP.O_I.handle = objectClass ;
	break ;

      case DDM_MODIFY_REGION:
	Header.bodySize = 1 ;
	Header.VP.ddm.region = region ;
	break ;

      default:
	D.Out(pdExcept, "Unknown type %d in WriteHeader.", Header.type);
	throw RTIinternalError("Unknown/Unimplemented type for Header.");
    }

    if (Header.bodySize == 0)
        socket->send(reinterpret_cast<unsigned char *>(&Header), sizeof(HeaderStruct));

    G.Out(pdGendoc,"exit  NetworkMessage::writeHeader");
    return Header.bodySize != 0 ;
}

} // namespace certi

// $Id: NetworkMessage_RW.cc,v 3.45 2008/03/06 16:33:10 rousse Exp $
