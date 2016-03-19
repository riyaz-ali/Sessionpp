#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <ctime>

#include "Session.h"

using namespace std;

/**
  * @function	Session::Session
  * @brief		Default constructor
  */
Session::Session(){
	//clean up _session_id buffer
	memset(_session_id, 0, SESSION_ID_LENGTH);

	//generate a random id
	generateSessionID(_session_id);

	//check if file already exists
	if( !fileExists(_session_id) ){
		//create a new file
		_FILE.open(_session_id, ios::out );
		_FILE.close();

		//reopen file with r-w permission
		_FILE.open(_session_id, (ios::out|ios::in|ios::binary) );

		if(!_FILE){
			//Failed to open file
			throw SessionException("Failed to open file", SessionErrorCodes::SESS_FAILED_TO_OPEN_FILE);
		}

	} else {
		throw SessionException("Session Already exists. Possible duplicate key generated.", SessionErrorCodes::SESS_ALREADY_CREATED);
	}
}

/**
  * @function	Session::Session
  * @brief		Secondary CTOR to resume an existing session identified by id
  */
Session::Session(const char* id){
	if(isValidID(id)){
		//transfer the id to _session_id
		strcpy(_session_id, id);

		//open the Session file
		_FILE.open(_session_id, (ios::in|ios::out|ios::binary) );
		
		if(!_FILE){
			//Failed to open file
			throw SessionException("Failed to open file", SessionErrorCodes::SESS_FAILED_TO_OPEN_FILE);
		}

		//read session contents to _session_map
		deSerialize();
		
	} else {
		//invalid id
		throw SessionException("Invalid Session ID", SessionErrorCodes::SESS_INVALID_ID);
	}
}

/**
  *	@function	Session::~Session
  * @brief		DTOR: Flushes content and close any handle
  */
Session::~Session(){
	if(_FILE.good()){
		flush();
		_FILE.close();
	}
}

/**
  * @function	Session::flush
  * @brief		Flushes the current session state into session file
  */
int Session::flush(){
	//the file should be 'good' for the operation
	if( !_FILE.good() )
		return SESS_FLUSH_FAILURE;


	//close nd reopen file with truncate
	_FILE.close();
	_FILE.clear();	//to clear associated flags
	_FILE.open(_session_id, (ios::out|ios::binary|ios::trunc) );	//trunc will erase the file!
	if(!_FILE){
		//Failed to open file
		return SESS_FLUSH_FAILURE;
	}
	return serialize(_FILE);	//this will flush all the data into the file
}

/**
  *	@function	Session::operator[]
  * @brief		Provides map-indexing and also adds session variables on the fly
  */
SessionData& Session::operator[](const char* key){
	_map_type::iterator it;
	it = _session_map.find(key);

	//if key is found
	if(it != _session_map.end())
		return it->second;

	//else if key is not found
	//create an empty key --the first val in return value contains iterator to inserted object
	it = _session_map.insert( _kv_map(key,"") ).first;

	return it->second;
}

/**
  *	@function	Session::opeartor<<
  * @brief		dumps session data to ostream
  */
ostream& operator<<(ostream& out, Session& sess){
	for(auto& const _sess_pair : sess._session_map){
		//output as --	'key' => 'value'
		out << "'" << _sess_pair.first << "' => '" << _sess_pair.second << "'" << endl;
	}
	return out;
}

/**
  * @function	Session::generatesessionID
  * @brief		Creates a randomm session id
  */
void Session::generateSessionID(char *dest){
	static int whichSet = 0;
	static bool unseeded = true;
	if(unseeded){
		srand(time(0));
		unseeded = false;
	}

	//having a variety of charset and changing them each tym while generating id will ensure low collision rate
	static char *charset[] = 
		{	
			"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
			"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz",
			"AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz9876543210"
		};

	size_t length = SESSION_ID_LENGTH;

	while ( length-- > 0) {
		size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
		*dest++ = charset[whichSet][index];
	}
	*dest = '\0';
	whichSet++;
	if(whichSet > 2)whichSet=0;
}

/**
  * @function	Session::fileExists
  * @brief		checks if a file with specified name exists or not
  */
bool Session::fileExists(const char* name){
	FILE *file = NULL;
	if( (file = fopen(name, "rb") ) ){
		fclose(file);
		return true;
	} else {
		return false;
	}
}

/**
  * @function	Session::isValidID
  * @brief		checks if the given id is a valid Session ID
  */
bool Session::isValidID(const char* id){
	if( strlen(id) != SESSION_ID_LENGTH )
		return false;

	//iterate through each character and check if they are alphanum or not
	while(*id){
		if(!isalnum(*id)) return false;
		id++;
	}
	return true;
}

/**
  *	@function	Session::serialize
  * @brief		serializes the content of the session so that it could be stored in a file
  */
int Session::serialize(ostream& buffer){
	int x = 0;	
	for(auto& const _sess_pair : _session_map){
		buffer << _sess_pair.first << "=" << _sess_pair.second << "&";
		x++;
	}
	return x;
}

/**
  * @function	Session::deSerialize
  * @brief		de-serializes contents from file to map
  */
void Session::deSerialize(){
	string line;
	size_t pos;
	while( getline(_FILE, line, '&') ){
		pos = line.find('=');
		string key = line.substr(0,pos);
		string val = line.substr(pos+1);	//till npos

		//push to map
		_session_map.insert(_kv_map(key,val));
	}
}





/*************************SESSION DATA**********************************/
ostream& operator<<(ostream& out, SessionData& s_data){
	return out << s_data._str;
}