#include "JSON.h"

/******************************************************************************
 *
 *  JSONArrayElement
 *
******************************************************************************/
	JSONArrayElement::JSONArrayElement(const char *newval){
		size_t vallen=strlen(newval)+1;
		obj=NULL;
		array=NULL;
		numval=0;
		val=(char *)malloc(sizeof(char)*vallen);
		memcpy(val,newval,vallen);
	}
	JSONArrayElement::JSONArrayElement(char *newval){
		size_t vallen=strlen(newval)+1;
		obj=NULL;
		array=NULL;
		numval=0;
		val=(char *)malloc(sizeof(char)*vallen);
		memcpy(val,newval,vallen);
	}
	JSONArrayElement::JSONArrayElement(double newval){
		obj=NULL;
		array=NULL;
		numval=newval;
		val=NULL;
	}
	JSONArrayElement::JSONArrayElement(JSONObj *newval){
		obj=newval;
		array=NULL;
		numval=0;
		val=NULL;
	}
	JSONArrayElement::JSONArrayElement(JSONArray *newval){
		obj=NULL;
		array=newval;
		numval=0;
		val=NULL;
	}
	JSONArrayElement::~JSONArrayElement(){
		if (val!=NULL) {
			free(val);
			val=NULL;
		}
		if (obj!=NULL) {
			delete obj;
			obj=NULL;
		}
		if (array!=NULL) {
			delete array;
			array=NULL;
		}
		numval=0;
	}
	char *JSONArrayElement::getValue(){
		if (obj!=NULL) {
			return "OBJECT";
		}
		if (array!=NULL) {
			return "ARRAY";
		}
		if (val!=NULL) {
			return val;
		}
		char *tempbuf=(char *)malloc(sizeof(char)*25); //big enough to hold a 64-bit int
#ifndef XCODE
		sprintf(tempbuf,"%s",String(numval, 2).c_str()); //sprintf %f is broken on phohon
#else
		sprintf(tempbuf,"%4.2f",numval); //sprintf %f is broken on phohon
#endif
		return tempbuf;
	}
	void JSONArrayElement::setValue(const char *newval){
		size_t vallen=strlen(newval);
		if (obj!=NULL) {
			delete obj;
			obj=NULL;
		}
		if (array!=NULL) {
			delete array;
			array=NULL;
		}
		if (val!=NULL) {
			free(val);
			val=NULL;
		}
		val=(char *)malloc(sizeof(char)*vallen);
		memcpy(val,newval,vallen);
	}
	void JSONArrayElement::setValue(char *newval){
		size_t vallen=strlen(newval);
		if (obj!=NULL) {
			delete obj;
			obj=NULL;
		}
		if (array!=NULL) {
			delete array;
			array=NULL;
		}
		if (val!=NULL) {
			free(val);
			val=NULL;
		}
		val=(char *)malloc(sizeof(char)*vallen);
		memcpy(val,newval,vallen);
	}
	void JSONArrayElement::setValue(JSONObj *newval){
		if (obj!=NULL) {
			delete obj;
			obj=NULL;
		}
		if (array!=NULL) {
			delete array;
			array=NULL;
		}
		if (val!=NULL) {
			free(val);
			val=NULL;
		}
		obj=newval;
	}
	void JSONArrayElement::setValue(JSONArray *newval){
		if (obj!=NULL) {
			delete obj;
			obj=NULL;
		}
		if (array!=NULL) {
			delete array;
			array=NULL;
		}
		if (val!=NULL) {
			free(val);
			val=NULL;
		}
		array=newval;
	}
	void JSONArrayElement::setValue(double newval){
		if (obj!=NULL) {
			delete obj;
			obj=NULL;
		}
		if (array!=NULL) {
			delete array;
			array=NULL;
		}
		if (val!=NULL) {
			free(val);
			val=NULL;
		}
		numval=newval;
	}
	char *JSONArrayElement::jstringify(){
		char *buf;
		int buflen=1; // long enough to hold static content - \0
		if (val!=NULL) {
			buflen+=strlen(val)+2;
			buf=(char *)malloc(sizeof(char)*buflen);
			//fprintf(stderr,"JSONElement::stringify malloced buf 0x%lx\n",(long)buf);
			sprintf(buf,"\"%s\"",val);
			return buf;
		}
		if (obj!=NULL) {
			char *tmpbuf=obj->jstringify();
            if (tmpbuf!=NULL) {
                buflen+=strlen(tmpbuf);
                buf=(char *)malloc(sizeof(char)*buflen);
                //fprintf(stderr,"JSONElement::stringify malloced buf 0x%lx\n",(long)buf);
                sprintf(buf,"%s",tmpbuf);
                free(tmpbuf);
                tmpbuf=NULL;
            } else {
                sprintf(buf,"%s","NULL");
                //fprintf(stderr,"JSONElement::stringify obj not null but obj->stringify returned NULL\n");
            }
			return buf;
		}
		if (array!=NULL) {
			char *tmpbuf=array->jstringify();
            if (tmpbuf!=NULL) {
                buflen+=strlen(tmpbuf);
                buf=(char *)malloc(sizeof(char)*buflen);
                //fprintf(stderr,"JSONElement::stringify malloced buf 0x%lx\n",(long)buf);
                sprintf(buf,"%s",tmpbuf);
                free(tmpbuf);
                tmpbuf=NULL;
            } else {
                sprintf(buf,"%s","NULL");
                //fprintf(stderr,"JSONElement::stringify obj not null but obj->stringify returned NULL\n");
            }
			return buf;
		}
		buf=(char *)malloc(sizeof(char)*25); //big enough to hold a 64-bit int
#ifndef XCODE
		sprintf(buf,"%s",String(numval, 2).c_str()); //sprintf %f is broken on phohon
#else
		sprintf(buf,"%4.2f",numval); //sprintf %f is broken on phohon
#endif
		return buf;
	}

/******************************************************************************
 *
 *  JSONArray
 *
******************************************************************************/
	JSONArray::JSONArray(){
		root=NULL;
		len=0;
	}
	JSONArray::JSONArray(char *doc) { //create new JSON array from JSON text
		root=NULL;
		len=0;
		if (doc==NULL || *doc!='[') { //could not parse string
			return;
		}
		char *pos=doc;
		//JSONElement *newElement;
		JSONArray *newArray;
		JSONObj *newObj;
		pos++;
		while(*pos!='\0' && *pos!=']') {
			char *token=_nexttoken(pos);
			pos+=strlen(token);
			switch(*token){
			case '"': //array element
				addElement(token);
				break;
			case '[': //array
				newArray=new JSONArray(token);
				addElement(newArray);
				break;
			case '{': //object
				newObj=new JSONObj(token);
				addElement(newObj);
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				addElement(atoi(token));
			}
			free(token);
		}
	}
	char *JSONArray::_nexttoken(char *data) {
		char *buf=(char *)malloc(sizeof(char)*(strlen(data)+1));
		char *bufpos=buf;
		char *pos=data;
        int nestcount=0;
		while ((*pos==' ' ||*pos=='\t' ||*pos=='\n' ||*pos=='\r' || *pos==',')) { //scan forward to start of token
			pos++;
			}
		while (((*pos!=',' && *pos!=']') || nestcount>0 ) && *pos!='\0') { // look for valid end token or end of string
			*bufpos=*pos;
            
            switch(*pos) {
                case '{':
                case '[':
                    nestcount++;
                    break;
                case '}':
                case ']':
                    nestcount--;
                    break;
                case '\\':
                    bufpos++;
                    pos++;
                    *bufpos=*pos;
            }

            
			bufpos++;
			pos++;
	
            
		}
		*bufpos='\0'; //terminate string
		return buf;
	}
	JSONArray::~JSONArray(){
		for (int i=0;i<len;i++) {
			delete root[i];
			root[i]=NULL;
		}
        free(root);
		root=NULL;
	}
	void JSONArray::addElement(JSONArrayElement *newval){
		if (root==NULL) {
			root=(JSONArrayElement **)malloc(sizeof(JSONArrayElement *));
			//fprintf(stderr,"Realloc JSONArray at 0x%lx from %d to %d\n",(long)root,len,len+1);

			len=1;
			root[0]=newval;
		} else {
			JSONArrayElement **newroot=(JSONArrayElement **)realloc(root,sizeof(JSONArrayElement *)*(len+1));
			//fprintf(stderr,"Realloc JSONArray at 0x%lx from %d to %d\n",(long)root,len,len+1);

            if (newroot==NULL) {
                return;
            }
            if (newroot!=root) {
                root=newroot;
            }
			root[len]=newval;
			len++;
		}
	}
	void JSONArray::addElement(const char *newval){
		JSONArrayElement *newEl=new JSONArrayElement(newval);
		addElement(newEl);
	}
	void JSONArray::addElement(char *newval){
		JSONArrayElement *newEl=new JSONArrayElement(newval);
		addElement(newEl);
	}
	void JSONArray::addElement(double newval){
		JSONArrayElement *newEl=new JSONArrayElement(newval);
		addElement(newEl);
	}
	void JSONArray::addElement(JSONObj *newval){
		JSONArrayElement *newEl=new JSONArrayElement(newval);
		addElement(newEl);
	}
	void JSONArray::addElement(JSONArray *newval){
		JSONArrayElement *newEl=new JSONArrayElement(newval);
		addElement(newEl);
	}
	char *JSONArray::jstringify(){
		char *buf;
		int buflen=len+3; // long enough to hold static content - []\0 and the commas
		int pos=0;
		buf=(char *)malloc(sizeof(char)*buflen);
		//fprintf(stderr,"Alloced buf 0x%lx\n",(long)buf);
		buf[pos]='[';
		pos++;
		for(int i=0;i<len;i++) {
			if(root[i]!=NULL) {
				if(i>0) { //if this not the first element
					buf[pos]=',';
					pos++;
					buflen++;
					char *newbuf=(char *)realloc(buf,sizeof(char)*buflen);
                    if (newbuf==NULL) {
                        //bLink->printDebug("Realloc Failed!");
                        sprintf(buf,"%s","NULL");
                        return buf;
                    }
                    if (newbuf!=buf) {
                        //free(buf);
                        buf=newbuf;
                    }

				}
				char *tempbuf=root[i]->jstringify();
				if (tempbuf!=NULL) {
				buflen+=strlen(tempbuf);
				buf[pos]='\0';
				//bLink->printDebug("stringifying JSONElement - %s pos=%d buflen=%d buf=%s",tempbuf,pos,buflen,buf);
				//fprintf(stderr,"stringifying JSONElement - %s pos=%d buflen=%d buf=%s\n",tempbuf,pos,buflen,buf);
				char *newbuf=(char *)realloc(buf,sizeof(char)*buflen);
                if (newbuf==NULL) {
                    //bLink->printDebug("Realloc Failed!");
                    //free(buf);
                    free(tempbuf);
                    sprintf(buf,"%s","NULL");

                    return buf;
                }
                if (newbuf!=buf) {
                    //free(buf);
                    buf=newbuf;
                }
				//fprintf(stderr,"Realloced buf 0x%lx\n",(long)buf);

				sprintf(&(buf[pos]),"%s",tempbuf);
				pos+=strlen(tempbuf);
				//fprintf(stderr,"Freeing tempbuf 0x%lx\n",(long)tempbuf);
				free(tempbuf);
				tempbuf=NULL;
				} /*else {
					//bLink->printDebug("Stringify failed!");
					fprintf(stderr,"Stringify failed\n");
				}*/
			} /*else {
				//bLink->printDebug("JSON document has NULL element!");
				fprintf(stderr,"JSON document has NULL element!\n");
			}*/
		}
		buf[pos]=']';
		pos++;
		buf[pos]='\0';
		return buf;
	}


/******************************************************************************
 *
 *  JSONElement
 *
******************************************************************************/
	JSONElement::JSONElement(char *doc) {
		char *pos=doc;
		char *tmpname=(char *)malloc(sizeof(char)*(strlen(doc)+1));
		char *namepos=tmpname;
		char *tmpval=(char *)malloc(sizeof(char)*(strlen(doc)+1));
		char *valpos=tmpval;
		obj=NULL;
		array=NULL;
		val=NULL;
		numval=0;
        size_t vallen;
		if (*pos!='\"') {
			free(tmpname);
			free(tmpval);
			return;
		}
		pos++;
		while(*pos!='"' && *pos!='\0'){
			*namepos=*pos;
			pos++;
			namepos++;
		}
        pos++; // go past close quote
		*namepos='\0';
		while (*pos==':' || *pos==' ' || *pos=='\t' || *pos=='\r' || *pos=='\n') {  // scan to start of value
			pos++;
		}
		switch(*pos) {
		case '"':
			pos++; //move past quote
			while(*pos!='"' && *pos!='\0'){
				*valpos=*pos;
				pos++;
				valpos++;
			}
			*valpos='\0';
            vallen=strlen(tmpval)+1;
            val=(char *)malloc(sizeof(char)*vallen);
            memcpy(val,tmpval,vallen);

			break;
		case '[': //array
			array=new JSONArray(pos);
			break;
		case '{': //object
			obj=new JSONObj(pos);
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			numval=atoi(pos);
			break;
		}
        size_t namelen=strlen(tmpname)+1;
        name=(char *)malloc(sizeof(char)*namelen);
        memcpy(name,tmpname,namelen);
        free(tmpname);
        free(tmpval);
    }

	JSONElement::JSONElement(const char *elementName,char *newval) {
		size_t namelen=strlen(elementName)+1;
		obj=NULL;
		array=NULL;
        val=NULL;
		numval=0;
		name=(char *)malloc(sizeof(char)*namelen);
		memcpy(name,elementName,namelen);
        if (newval!=NULL) {
            size_t vallen=strlen(newval)+1;
            val=(char *)malloc(sizeof(char)*vallen);
            memcpy(val,newval,vallen);
        }
		//bLink->printDebug("JSONElement[%s]=%s",name,val);
		//fprintf(stderr,"JSONElement[%s]=%s\n",name,val);

	}

	JSONElement::JSONElement(const char *elementName,const char *newval) {
        size_t namelen=strlen(elementName)+1;
        obj=NULL;
        array=NULL;
        val=NULL;
        numval=0;
        name=(char *)malloc(sizeof(char)*namelen);
        memcpy(name,elementName,namelen);
        if (newval!=NULL) {
            size_t vallen=strlen(newval)+1;
            val=(char *)malloc(sizeof(char)*vallen);
            memcpy(val,newval,vallen);
        }
		//bLink->printDebug("JSONElement[%s]=%s",name,val);
		//fprintf(stderr,"JSONElement[%s]=%s\n",name,val);

	}
	JSONElement::JSONElement(const char *elementName,double newval) {
		size_t namelen=strlen(elementName)+1;
		obj=NULL;
		name=(char *)malloc(sizeof(char)*namelen);
		memcpy(name,elementName,namelen);
		//val=(char *)malloc(sizeof(char)*11); //2^32 is 10 chars long
		//sprintf(val,"%d",newval);
		//bLink->printDebug("JSONElement[%s]=%s",name,val);
		//fprintf(stderr,"JSONElement[%s]=%s\n",name,val);
		val=NULL;
		array=NULL;
		numval=newval;

	}
	JSONElement::JSONElement(const char *elementName,JSONObj *newval) {
		size_t namelen=strlen(elementName)+1;
		obj=newval;
		val=NULL;
		array=NULL;
		numval=0;
		name=(char *)malloc(sizeof(char)*namelen);
		memcpy(name,elementName,namelen);
		//bLink->printDebug("JSONElement[%s]=OBJECT 0x%x",name,(int)obj);
		//fprintf(stderr,"JSONElement[%s]=%lx\n",name,(long)obj);

	}

	JSONElement::JSONElement(const char *elementName,JSONArray *newval) {
		size_t namelen=strlen(elementName)+1;
		obj=NULL;
		val=NULL;
		array=newval;
		numval=0;
		name=(char *)malloc(sizeof(char)*namelen);
		memcpy(name,elementName,namelen);
		//bLink->printDebug("JSONElement[%s]=OBJECT 0x%x",name,(int)obj);
		//fprintf(stderr,"JSONElement[%s]=%lx\n",name,(long)obj);

	}
	JSONElement::~JSONElement() {
		if (val!=NULL) {
			free( val);
			val=NULL;
		}
		if (name!=NULL) {
			free(name);
			name=NULL;
		}
		if (obj!=NULL) {
			delete obj;
			obj=NULL;
		}
		if (array!=NULL) {
			delete array;
			array=NULL;
		}
	}
	char *JSONElement::getName() {
		return name;
	}
	char *JSONElement::getValue() {
		if (obj!=NULL) {
			return "OBJECT";
		}
		if (array!=NULL) {
			return "ARRAY";
		}
		if (val) {
			return val;
		}
		char *tempbuf=(char *)malloc(sizeof(char)*25); //big enough to hold a 64-bit int
#ifndef XCODE
		sprintf(tempbuf,"%s",String(numval, 2).c_str()); //sprintf %f is broken on phohon
#else
		sprintf(tempbuf,"%4.2f",numval); //sprintf %f is broken on phohon
#endif
		return tempbuf;
	}
	int JSONElement::getValueAsInt() {
		if (obj!=NULL) {
			return 0;
		}
		if (val!=NULL) {
			return atoi(val);

		}
		return 0;
	}
	void JSONElement::setValue(char *newval){
		size_t vallen=strlen(newval);
		if (obj!=NULL) {
			delete obj;
			obj=NULL;
		}
		if (array!=NULL) {
			delete array;
			array=NULL;
		}
		if (val!=NULL) {
			free(val);
			val=NULL;
		}
		val=(char *)malloc(sizeof(char)*vallen);
		memcpy(val,newval,vallen);
	}
	void JSONElement::setValue(JSONObj *newval) {
		if (obj!=NULL) {
			delete obj;
			obj=NULL;
		}
		if (array!=NULL) {
			delete array;
			array=NULL;
		}
		if (val!=NULL) {
			free(val);
			val=NULL;
		}
		obj=newval;
	}
	void JSONElement::setValue(JSONArray *newval) {
		if (obj!=NULL) {
			delete obj;
			obj=NULL;
		}
		if (array!=NULL) {
			delete array;
			array=NULL;
		}
		if (val!=NULL) {
			free(val);
			val=NULL;
		}
		array=newval;
	}
	void JSONElement::setValue(double newval){
		if (obj!=NULL) {
			delete obj;
			obj=NULL;
		}
		if (array!=NULL) {
			delete array;
			array=NULL;
		}
		if (val!=NULL) {
			free(val);
			val=NULL;
		}
		//val=(char *)malloc(sizeof(char)*11); //2^32 is 10 chars long
		//sprintf(val,"%d",newval);
		numval=newval;
	}
	char *JSONElement::jstringify(){
		char *buf;
		int buflen=4; // long enough to hold static content - "":""\0
        if (name==NULL) {
            name=(char *)malloc(sizeof(char)*5);
            sprintf(name,"NULL");
        }
		buflen+=strlen(name);
		if (val!=NULL) {
			buflen+=strlen(val)+2;
			buf=(char *)malloc(sizeof(char)*buflen);
			//fprintf(stderr,"JSONElement::stringify malloced buf 0x%lx\n",(long)buf);
			sprintf(buf,"\"%s\":\"%s\"",name,val);
			return buf;
		}
		if (obj!=NULL) {
			char *tmpbuf=obj->jstringify();
            if (tmpbuf!=NULL) {
                buflen+=strlen(tmpbuf);
                buf=(char *)malloc(sizeof(char)*buflen);
                //fprintf(stderr,"JSONElement::stringify malloced buf 0x%lx\n",(long)buf);
                sprintf(buf,"\"%s\":%s",name,tmpbuf);
                free(tmpbuf);
                tmpbuf=NULL;
            } else {
                sprintf(buf,"%s","NULL");
                //fprintf(stderr,"JSONElement::stringify obj not null but obj->stringify returned NULL\n");
            }
			return buf;
		}
		if (array!=NULL) {
			char *tmpbuf=array->jstringify();
            if (tmpbuf!=NULL) {
                buflen+=strlen(tmpbuf);
                buf=(char *)malloc(sizeof(char)*buflen);
                //fprintf(stderr,"JSONElement::stringify malloced buf 0x%lx\n",(long)buf);
                sprintf(buf,"\"%s\":%s",name,tmpbuf);
                free(tmpbuf);
                tmpbuf=NULL;
            } else {
                sprintf(buf,"%s","NULL");
                //fprintf(stderr,"JSONElement::stringify obj not null but obj->stringify returned NULL\n");
            }
			return buf;
		}
		buf=(char *)malloc(sizeof(char)*25); //big enough to hold a 64-bit int
#ifndef XCODE
		sprintf(buf,"\"%s\":%s",name,String(numval, 2).c_str()); //sprintf %f is broken on phohon
#else
		sprintf(buf,"\"%s\":%4.2f",name,numval); //sprintf %f is broken on photon
#endif
		return buf;
	}


/******************************************************************************
*
*  JSONObject
*
******************************************************************************/



	JSONObj::JSONObj() {
		root=NULL;
		len=0;
	}
	JSONObj::JSONObj(char *doc) { //create new JSON object from JSON text
		root=NULL;
		len=0;
		if (doc==NULL || *doc!='{') { //could not parse string
			return;
		}
		char *pos=doc;
		JSONElement *newElement;
		//JSONArray *newArray;
		//JSONObj *newObj;
		pos++;
		while(*pos!='\0' && *pos!='}') {
			char *token=_nexttoken(pos);
			pos+=strlen(token);
			switch(*token){
			case '"': //element
				newElement=new JSONElement(token);
				addElement(newElement);
				break;
			/*case '[': //array
				newArray=new JSONArray(token);
				addElement(newArray);
				break;*/
			/*case '{': //object
				newObj=new JSONObj(token);
				addElement(newObj);
				break;*/
			}
			free(token);
		}
	}
	char *JSONObj::_nexttoken(char *data) {
		char *buf=(char *)malloc(sizeof(char)*(strlen(data)+1));
		char *bufpos=buf;
		char *pos=data;
        int nestcount=0;
		while ((*pos==' ' ||*pos=='\t' ||*pos=='\n' ||*pos=='\r' || *pos==',')) { //scan forward to start of token
			pos++;
			}
		while (((*pos!=',' && *pos!=']') || nestcount>0) && *pos!='\0') {
         	*bufpos=*pos;
            switch(*pos) {
                case '{':
                case '[':
                    nestcount++;
                    break;
                case '}':
                case ']':
                    nestcount--;
                    break;
                case '\\':
                    bufpos++;
                    pos++;
                    *bufpos=*pos;
            }
            
			bufpos++;
			pos++;
            
	
		}
        *bufpos=*pos;
        if (*pos!='\0') {
            bufpos++;
            *bufpos='\0'; //terminate string
        }
		return buf;
	}

	JSONObj::~JSONObj() {
		for (int i=0;i<len;i++) {
			delete root[i];
			root[i]=NULL;
		}
        free(root);
		root=NULL;
	}
	JSONElement *JSONObj::getElement(char *name) {
		for (int i=0;i<len;i++) {
			if(!strcmp(name,root[i]->getName())) {
				return root[i];
			}
		}
		return NULL; //no such element found
	}
	JSONElement *JSONObj::getFirstElement() {
		if (len>0) {
				return root[0];
		}
		return NULL; //no such element found
	}
	void JSONObj::addElement(JSONElement *newElement) {
		if (root==NULL) {
			//bLink->printDebug("Realloc JSON at 0x%x from %d to %d",(int)root,len,len+1);
			//fprintf(stderr,"Realloc JSONObj at 0x%lx from %d to %d\n",(long)root,len,len+1);

			root=(JSONElement **)malloc(sizeof(JSONElement *));
			len=1;
			root[0]=newElement;
		} else {
			//bLink->printDebug("Realloc JSON at 0x%x from %d to %d",(int)root,len,len+1);
			//fprintf(stderr,"Realloc JSONObj at 0x%lx from %d to %d\n",(long)root,len,len+1);
			JSONElement **newroot=(JSONElement **)realloc(root,sizeof(JSONElement *)*(len+1));
            if (newroot==NULL) {
                //bLink->printDebug("Realloc Failed!");
                return;
            }
            if (newroot!=root) {
                //free(root);
                root=newroot;
            }
			root[len]=newElement;
			len++;
		}
	}

	void JSONObj::addElement(const char *elementName,char *newval) {
		JSONElement *newEl=new JSONElement(elementName,newval);
		addElement(newEl);
	}
	void JSONObj::addElement(const char *elementName,double newval) {
		JSONElement *newEl=new JSONElement(elementName,newval);
		addElement(newEl);
	}

	void JSONObj::addElement(const char *elementName,JSONObj *newval) {
		JSONElement *newEl=new JSONElement(elementName,newval);
		addElement(newEl);
	}
	void JSONObj::addElement(const char *elementName,JSONArray *newval) {
		JSONElement *newEl=new JSONElement(elementName,newval);
		addElement(newEl);
	}

	char *JSONObj::jstringify(){
		char *buf;
		int buflen=len+3; // long enough to hold static content - {}\0 and the commas
		int pos=0;
		buf=(char *)malloc(sizeof(char)*buflen);
		//fprintf(stderr,"Alloced buf 0x%lx\n",(long)buf);
		buf[pos]='{';
		pos++;
		for(int i=0;i<len;i++) {
			if(root[i]!=NULL) {
				if(i>0) { //if this not the first element
					buf[pos]=',';
					pos++;
					buflen++;
					char *newbuf=(char *)realloc(buf,sizeof(char)*buflen);
                    if (newbuf==NULL) {
                        //bLink->printDebug("Realloc Failed!");
                        //free(buf);
                        sprintf(buf,"%s","NULL");
                        return buf;
                    }
                    if (newbuf!=buf) {
                        //free(buf);
                        buf=newbuf;
                    }

				}
				char *tempbuf=root[i]->jstringify();
				if (tempbuf!=NULL) {
				buflen+=strlen(tempbuf);
				buf[pos]='\0';
				//bLink->printDebug("stringifying JSONElement - %s pos=%d buflen=%d buf=%s",tempbuf,pos,buflen,buf);
				//fprintf(stderr,"stringifying JSONElement - %s pos=%d buflen=%d buf=%s\n",tempbuf,pos,buflen,buf);
				char *newbuf=(char *)realloc(buf,sizeof(char)*buflen);
                if (newbuf==NULL) {
                    //bLink->printDebug("Realloc Failed!");
                    //free(buf);
                    sprintf(buf,"%s","NULL");
                    return buf;
                }
                if (newbuf!=buf) {
                    //free(buf);
                    buf=newbuf;
                }
				//fprintf(stderr,"Realloced buf 0x%lx\n",(long)buf);

				sprintf(&(buf[pos]),"%s",tempbuf);
				pos+=strlen(tempbuf);
				//fprintf(stderr,"Freeing tempbuf 0x%lx\n",(long)tempbuf);
				free(tempbuf);
				tempbuf=NULL;
				} /*else {
					//bLink->printDebug("Stringify failed!");
					fprintf(stderr,"Stringify failed\n");
				}*/
			} /*else {
				//bLink->printDebug("JSON document has NULL element!");
				fprintf(stderr,"JSON document has NULL element!\n");
			}*/
		}
		buf[pos]='}';
		pos++;
		buf[pos]='\0';
		return buf;
	}
