#include "RoomMemberRequestHandler.h"

bool RoomMemberRequestHandler::isRequestRelevant(const RequestInfo& requestInfo)
{
    return requestInfo.messageCode <= 1 || requestInfo.messageCode >= 0;
}

RequestResult RoomMemberRequestHandler::leaveRoom(RequestInfo)
{
	m_roomManager.getRoom(m_room.getRoomData().id).removeUser(m_user.getUsername());

	RequestResult r;
	LeaveRoomResponse lrr;
	lrr.status = leaveRoomSuccessful;
	r.newHandler = RequestHandlerFactory::getInstance().createMenuRequestHandler(m_user.getUsername());
	r.response = JsonResponsePacketSerializer::serializeResponse(lrr);
	return r;
}

template <typename T>
bool contains(std::vector<T> vec, const T& elem)
{
	bool result = false;
	if (find(vec.begin(), vec.end(), elem) != vec.end())
	{
		result = true;
	}
	return result;
}

RequestResult RoomMemberRequestHandler::getRoomsState(RequestInfo)
{
	try{
		if (!contains(m_roomManager.getRoom(m_room.getRoomData().id).getAllUsers(), m_roomManager.getRoom(m_room.getRoomData().id).getRoomData().adminName))
		{
			throw 69;
		}
		RequestResult r;
		GetRoomStateResponse grsr;
		grsr.answerTimeout = m_roomManager.getRoom(m_room.getRoomData().id).getRoomData().timePerQuestion;
		grsr.hasGameBegun = m_roomManager.getRoom(m_room.getRoomData().id).getRoomData().isActive;
		grsr.questionCount = m_roomManager.getRoom(m_room.getRoomData().id).getRoomData().numOfQuestionsInGame;
		grsr.players = m_roomManager.getRoom(m_room.getRoomData().id).getAllUsers();
		grsr.status = getRoomsStateRes;
		if (m_roomManager.getRoom(m_room.getRoomData().id).getRoomData().isActive != 0)
		{
			//!!!!!
			//!!!!!
			r.newHandler = nullptr; //later this will point to handler for game
			//!!!!!
			//!!!!!

		}
		else {
			r.newHandler = RequestHandlerFactory::getInstance().createRoomMemberRequestHandler(m_user, m_roomManager.getRoom(m_room.getRoomData().id));
		}
			r.response = JsonResponsePacketSerializer::serializeResponse(grsr);
		return r;
    }
	catch (...)
	{
		RequestResult r;
		ErrorResponse e;
		e.message = "room closed";
		r.response = JsonResponsePacketSerializer::serializeResponse(e);
		r.newHandler = RequestHandlerFactory::getInstance().createMenuRequestHandler(m_user.getUsername());
		return r;
	}
}

RoomMemberRequestHandler::RoomMemberRequestHandler(std::string username, Room room) : m_room(room), m_user(username), m_roomManager(RequestHandlerFactory::getInstance().getRoomManager())
{
}

RequestResult RoomMemberRequestHandler::handleRequest(const RequestInfo& requestInfo)
{
	if (isRequestRelevant(requestInfo))
	{
		switch (requestInfo.messageCode)
		{
		case LeaveRoom:
			return this->leaveRoom(requestInfo);
		case GetRoomsState:
			return this->getRoomsState(requestInfo);
		}
	}
	RequestResult r;
	ErrorResponse e;
	e.message = "irrelevant message";
	r.response = JsonResponsePacketSerializer::serializeResponse(e);
	r.newHandler = RequestHandlerFactory::getInstance().createMenuRequestHandler(m_user.getUsername());
	return r;
}
