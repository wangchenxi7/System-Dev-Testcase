/**
 *  C++ non-virtual function override testcase.
 * 
 */ 


#include "stdio.h"

class father{

public:
	int field;

	father(){
		field = 0;
	}

	char* name()	{ 
			char *name_str = "I'm father";		
			return name_str ;  
	}

	virtual char* status(){
		char* status_str="I'm fathering.";
		return status_str;
	}

};

class son: public father{
public:
	int field;

	son(){
		field =1;
	}

		char* name()	{ 
			char *name_str = "I'm son";		
			return name_str ;  
	}

	virtual char* status(){
		char* status_str="I'm sonning.";
		return status_str;
	}

};



int main(){
	class father *instance_father = new father();
	class son    *instance_son		= new	son();

	printf(" father : \n");
	printf("		field: %d, name : %s , status : %s \n", 
											instance_father->field, instance_father->name(),instance_father->status() );

	printf(" son : \n");
	printf("		field: %d, name : %s , status : %s \n", 
											instance_son->field, instance_son->name(), instance_son->status());

	printf("Let father points to son's instance : \n");
	instance_father = instance_son;

	printf(" pointted to son_instance's father : \n");
	printf("		field: %d, name : %s, status : %s \n", 
											instance_father->field, instance_father->name(), instance_father->status());
	
	return 0;
}