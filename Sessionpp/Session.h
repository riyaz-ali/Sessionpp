#ifndef SESSION_H
#define SESSION_H

#include <fstream>
#include <map>
#include <string>
#include <exception>

using namespace std;


//user configurable length of SESSION_ID
#ifndef SESSION_ID_LENGTH
#define SESSION_ID_LENGTH 10
#endif

#if SESSION_ID_LENGTH < 5
#error	SESSION_ID_LENGTH too low!
#elif SESSION_ID_LENGTH > 100
#error	SESSION_ID_LENGTH too high!
#endif


/**
  * @enum	SessionErrorCodes
  */
typedef enum {
	SESS_OK				= 100,
	SESS_FAILED_TO_OPEN_FILE,
	SESS_ALREADY_CREATED,
	SESS_INVALID_ID,
	SESS_DATA_CORRUPTED,
	SESS_FLUSH_FAILURE
}SessionErrorCodes;

/**
  * @class	SessionException
  * @brief	The Exception class used by the Session		--for convenience only
  */
class SessionException : public exception {
	SessionErrorCodes _ec;
public:
	SessionException(const char* msg, SessionErrorCodes ec) : exception(msg), _ec(ec){}
	SessionErrorCodes error_code(){ return _ec;}
};


/**
  *	@class	The SessionData class
  * @brief	this class extends upon the base string class to provide more robust operations on various data types
  */
class SessionData {
	string _str;
public:
	SessionData(const char* _data): _str(_data) {}
	SessionData(const string& _data): _str(_data) {}
	SessionData(){}

	template<class _data_type>
	SessionData& operator=(_data_type _data){
		this->_str += std::to_string(_data);
		return *this;
	}

	//specialised form for operator=
	SessionData& operator=(const char* _data){
		this->_str = _data;
		return *this;
	}

	friend ostream& operator<<(ostream&, SessionData&);
};


/**
 *	@class	Session
 *	@brief	The Base Session Class
 */
class Session
{
	//Convience definition
	typedef pair<string, SessionData> _kv_map;
	typedef map<string, SessionData> _map_type;

	fstream _FILE;				//the session' file where data is stored when session is deleted from memory
	_map_type _session_map;		//map to hold data while session is alive


	char _session_id[SESSION_ID_LENGTH];	//Unique session id	--default 10 chars

public:
	/**
	  *	Default CTOR --creates a new session
	  */
	Session();

	/**
	  * Secondary CTOR --restores a session specified by ID
	  * @throws if --ID is invalid, session doesn't exists or is already running
	  */
	Session(const char* id);


	/**
	  * Default DTOR --saves session data to file identified by session_id
	  */
	~Session();


	/***********************UTILITY FUNCTION***********************/

	/**
	  * @function	flush
	  * @brief		flushes the current content of session to the file
	  */
	int flush();

	/**
	  *	@function	opertor[]
	  * @brief		Default Indexer --use it to look for a key in session or even create a new one
	  */
	SessionData& operator[](const char*);

	/**
	  *	@function	operator<<
	  * @brief		dumps the session map data into human readable form
	  */
	friend ostream& operator<<( ostream&, Session&);

	/**
	  *	@function	unset
	  * @brief		unsets the session while keeping it alive
	  */
	inline void unset(){
		_session_map.clear();
	}

	/**
	  * @function	getID
	  * @brief		returns the session's ID		--DO NOT MODIFY IT
	  */
	inline const char* getID(){
		return _session_id;
	}


private:
	/**
	  * @function	generateSessionID
	  * @link: http://stackoverflow.com/a/15768317
	  */
	static void generateSessionID(char *dest);

	/**
	  * @function	fileExists
	  */
	bool fileExists(const char*);

	/**
	  * @function	isValidID
	  */
	bool isValidID(const char*);

	/**
	  * @function	serialize
	  */
	int serialize(ostream&);

	/**
	  * @function	deSerialize
	  */
	void deSerialize();
};



#endif	//SESSION_H