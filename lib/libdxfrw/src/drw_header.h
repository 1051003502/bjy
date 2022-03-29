 #ifndef DRW_HEADER_H
#define DRW_HEADER_H


#include <map>
#include "drw_base.h"

class dxfReader;
class dxfWriter;
class dwgBuffer;

#define SETHDRFRIENDS   \
    friend class dxfRW; \
    friend class DwgReader;

//! Class to handle header entries
/*!
*  Class to handle header vars, to read iterate over "std::map vars"
*  to write add a DRW_Variant* into "std::map vars" (do not delete it, are cleared in dtor)
*  or use add* helper functions.
*  @author Rallaz
*/
class DRW_Header {
    SETHDRFRIENDS
public:
    DRW_Header();
    ~DRW_Header() {
        clearVars();
    }

    DRW_Header(const DRW_Header& h){
        this->version = h.version;
        this->comments = h.comments;
		for(auto it = h.vars.begin(); it != h.vars.end(); ++it){
            this->vars[it->first] = new DRW_Variant( *(it->second) );
        }
        this->curr = NULL;
    }
    DRW_Header& operator=(const DRW_Header &h) {
       if(this != &h) {
           clearVars();
           this->version = h.version;
           this->comments = h.comments;
			for(auto it = h.vars.begin(); it != h.vars.end(); ++it){
               this->vars[it->first] = new DRW_Variant( *(it->second) );
           }
       }
       return *this;
    }

    void addDouble(std::string key, double value, int code);
    void addInt(std::string key, int value, int code);
    void addStr(std::string key, std::string value, int code);
    void addCoord(std::string key, DRW_Coord value, int code);
    std::string getComments() const {return comments;}
    void write(dxfWriter *writer, DRW::Version ver);
    void addComment(std::string c);

protected:
    void parseCode(int code, dxfReader *reader);
    bool parseDwg(DRW::Version version, dwgBuffer *buf, dwgBuffer *hBbuf, duint8 mv=0);
private:
    bool getDouble(std::string key, double *varDouble);
    bool getInt(std::string key, int *varInt);
    bool getStr(std::string key, std::string *varStr);
	
    bool getCoord(std::string key, DRW_Coord *varStr);
	
    void clearVars(){
		for (auto& var : vars)
			delete var.second;

        vars.clear();
    }

public:
    std::map<std::string,DRW_Variant*> vars;

private:
    std::string comments;
    std::string name;
    DRW_Variant* curr;
    int version; //to use on read

    duint32 linetypeCtrl;    /* line type control handle */
    duint32 layerCtrl;       /* layer control handle */
    duint32 styleCtrl;       /* style control handle */
    duint32 dimstyleCtrl;    /* dimension control handle */
    duint32 appidCtrl;       /* appid control handle */
    duint32 blockCtrl;       /* block control handle */
    duint32 viewCtrl;        /* view control handle */
    duint32 ucsCtrl;         /* ucs control handle */
    duint32 vportCtrl;       /* viewport control handle */
    duint32 vpEntHeaderCtrl; /* vpEntHeader control handle */
};

#endif
// EOF

