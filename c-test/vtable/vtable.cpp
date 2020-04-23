#include "stdio.h"


class klass{
public:
	int a;
	virtual int  oop_size() {
		return 1;
	}

};


class InstanceKlass : public klass{
public:

	int b;
	virtual int oop_size(){
		return 2;
	}

};


int main(void){
	class InstanceKlass* tmp_sub  = new InstanceKlass();
	class klass *tmp_base = tmp_sub;

	int oop_size = tmp_sub->oop_size();
	printf("tmp_sub->oop_size() %d \n", oop_size);

	oop_size = tmp_base->oop_size();
	printf("tmp_base->oop_size() %d \n", oop_size);

	return 0;

}