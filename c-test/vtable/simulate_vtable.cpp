#include "stdio.h"

class klass;
class InstanceKlass;
class InstanceMirrorKlass;

// Base Class
class klass{
public:
	int flag;

	klass(){
		flag = 0;
	}

	int  oop_size() {

		// Empty.
		printf("This is the base class \n");

		// switch (flag)
		// {
		// case 0:
		// 	printf("This is the base klass \n");
		// 	break;
		
		// case 1: 
		// 	((class InstanceKlass*)this)->oop_size();
		// 	break;
		// case 2:
		// 	((class InstanceMirrorKlass*)this)->oop_size();
		// 	break;

		// default:
		// 	printf("Wrong value\n");

		// 	break;
		// } 


	}

};


class InstanceKlass : public klass{
public:

	int b;

	InstanceKlass(){
		flag = 1;
	}

	int oop_size(){
		printf("This is InstanceKlass \n");
	}

};





class InstanceMirrorKlass : public klass{
public:

	int b;

	InstanceMirrorKlass(){
		flag = 2;
	}

	int oop_size(){
		printf("This is InstanceMirrorKlass \n");
	}

};











int main(void){
	class klass *tmp_base;
	class InstanceKlass* tmp_sub  = new InstanceKlass();
	class InstanceMirrorKlass* tmp_sub2 = new InstanceMirrorKlass();

	// #1 InstanceKlass 
	printf("#1 InstanceKlass  \n");
	tmp_sub->oop_size();
	
	// #2, Base class points to InstanceKlass
	printf("#2, Base class points to InstanceKlass \n");
	tmp_base = tmp_sub;
	if(tmp_base->flag == 1)
		((class InstanceKlass*)tmp_base)->oop_size();
	else if(tmp_base->flag == 2)
		((class InstanceMirrorKlass*)tmp_base)->oop_size();
	else
		tmp_base->oop_size();
	

	// #3, InstanceMirrorKlass
	printf("#3, InstanceMirrorKlass \n");
	tmp_sub2->oop_size();

	// #4, Base class points to InstanceMirroeKlas
	printf("#4, Base class points to InstanceMirroeKlas \n");
	tmp_base = tmp_sub2;
	if(tmp_base->flag == 1)
		((class InstanceKlass*)tmp_base)->oop_size();
	else if(tmp_base->flag == 2)
		((class InstanceMirrorKlass*)tmp_base)->oop_size();
	else
		tmp_base->oop_size();


	return 0;

}