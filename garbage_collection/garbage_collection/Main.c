#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DEFAULT_NUM_OF_MAX_OBJECTS 5
#define DEF_NUM_OF_MAX_POINTERS 256

typedef enum {
	int_object,
	pair_object
} ObjectType;

typedef struct sObject {
	ObjectType type;
	size_t marked; // for mark of all elements in stack
	struct sObject* next;
	union { // our object can has value or two pointer
		int value;
		struct {
			struct sObject* head; 
			struct sObject* tail; 
		};
	};
} Object;

typedef struct stack {
	// our stack will have array of pointer to objects
	Object* first; // pointer that point to the first element of linked list(LL) that contain all elements of universe
	Object* stack[DEF_NUM_OF_MAX_POINTERS]; // it's array of pointer that can point to every object in our universe
	size_t max_objects;
	size_t used_objects;
} VirtualMachine;

VirtualMachine* new_stack()
{
	VirtualMachine* new_vm = (VirtualMachine*)malloc(sizeof(VirtualMachine));
	if (new_vm) {
		new_vm->used_objects = 0;
		new_vm->first = NULL;
		new_vm->max_objects = DEFAULT_NUM_OF_MAX_OBJECTS;
		new_vm->stack[new_vm->used_objects] = new_vm->first;
	}
	return new_vm;
}

size_t random() {
	srand(time(NULL));
	size_t myInt = 1 + rand() % 100;
	return myInt;
}

Object* pop(VirtualMachine* vm) {
	if (vm->used_objects > 0) {
		return vm->stack[--vm->used_objects];
	}
	else {
		printf("Stack underflow\n");
		return NULL;
	}
}

void push__object__to_stack(VirtualMachine* vm, Object* object) {
	object->marked = 1;
	vm->stack[vm->used_objects++] = object;	

	//if (object->type == pair_object) free(vm->stack[vm->used_objects]), vm->stack[vm->used_objects] = NULL;
	// я добавляю их в вертикальный список в запвисимости от числа объектов.
	// то есть каждый новый объект добавляется на новой строке.
}

void mark(Object* object) {
	if (object->marked == 1) return;

	if (object->type == int_object) {
		object->marked = 1;
	}
	else if (object->type == pair_object) {
		mark(object->head);
		mark(object->tail);
	}
}

void markall(VirtualMachine* vm) {
	for (size_t i = 0; i < vm->used_objects; i++) {
		mark(vm->stack[i]);
	}
}
 
void sweep(VirtualMachine* vm) {
	Object* revisor = vm->first;
	int count_of_leak_memory_object = 0;
	Object* delete_from_universe;
	size_t mark = revisor->marked;
	while (revisor) {
		if (revisor->marked == 0) {
			delete_from_universe = revisor;
			revisor = delete_from_universe->next;
			free(delete_from_universe);
			vm->first = revisor;
			vm->used_objects--;
			count_of_leak_memory_object++;
		}
		else {
			if (revisor->type == pair_object) {
				while (revisor->head->type == pair_object || revisor->tail->type == pair_object)
				{
					if (revisor->head->type == pair_object) revisor = revisor->head, revisor->marked = 0;
					if (revisor->tail->type == pair_object) revisor = revisor->tail, revisor->marked = 0;
				}
			}
			revisor = revisor->next;
		}
	}
	printf("Number of unused deleted object is equal to: %d", count_of_leak_memory_object);
}

void gc(VirtualMachine* vm) {
	//markall(vm); // set "marked" parametr of all element to 1
	sweep(vm); // cleanup in our Virtaul Machine

	vm->max_objects = vm->used_objects == 0 ? DEFAULT_NUM_OF_MAX_OBJECTS : vm->used_objects * 2;
}

Object* object_create(VirtualMachine* vm, ObjectType type) {
	Object* new_object = (Object*)malloc(sizeof(Object));
	if (new_object) {
		new_object->type = type;
		new_object->marked = 0;
	}
	return new_object;
}

void set_first_pointer_of_LL_to_new_obj(VirtualMachine* vm, Object* object) {
	object->next = vm->first; // setting --first pointer-- of linked list to --new object--
	vm->first = object;
}

void printing_elements(Object* first) {
	if (first->type == pair_object) {
		printf("pair:\n");
		printing_elements(first->tail);
		printing_elements(first->head);
	}
	else if (first->type == int_object) {
		printf("object with value: %d\n", first->value);
	}
}

void object_print(Object* first) {

	switch(first->type) {
		case int_object:
			if (first->marked == 0) return;
			first->marked = 0;
			printf("%d", first->value);
			break;
		case pair_object: 
			printf("(");
			object_print(first->head);
			printf(", ");
			object_print(first->tail);
			printf(")");
			break;
	}
}

void adding_int_object(VirtualMachine* vm, int element) {
	Object* object;
	if (vm->used_objects == vm->max_objects) gc(vm);
	object = object_create(vm, int_object);
	object->value = element; // setting certain int_value to our object
	push__object__to_stack(vm, object);
	set_first_pointer_of_LL_to_new_obj(vm, object); // setting --first pointer-- of linked list to --new object--
	printf("We add int_object with value: %d\n\n", object->value);
}

void adding_pair_object(VirtualMachine* vm) {
	Object* object;
	printf("Now we will connect two previous objects to one!\n");
	object = object_create(vm, pair_object); // pair of two pointer: tail and head
	object->tail = pop(vm);
	printf("\nPrinting elements from tail pointer\n");
	printing_elements(object->tail);
	object->head = pop(vm);
	printf("Printing elements from head pointer\n");
	printing_elements(object->head);
	printf("\n");
	push__object__to_stack(vm, object);
	set_first_pointer_of_LL_to_new_obj(vm, object);
}

void adding_leak_memory_object(VirtualMachine* vm, int element) {
	Object* object;
	if (vm->used_objects == vm->max_objects) gc(vm);
	object = object_create(vm, int_object);
	object->value = element; // setting certain int_value to our object
	set_first_pointer_of_LL_to_new_obj(vm, object); // setting --first pointer-- of linked list to --new object--
	printf("We add int_object with value: %d\n\n", object->value);
	vm->used_objects++;
}

void freeing(VirtualMachine* vm) {
	Object* to_free = vm->first;
	Object* next;
	while (to_free) {
		next = to_free->next;
		free(to_free);
		to_free = next;
	}
}

int main() {
	Object* print;
	VirtualMachine *VM = new_stack();
	adding_int_object(VM, 1);
	adding_int_object(VM, 2);
	adding_pair_object(VM);
	adding_int_object(VM, 3);
	adding_pair_object(VM);
	adding_int_object(VM, 4);
	adding_pair_object(VM);
	adding_int_object(VM, 5);

	adding_leak_memory_object(VM, 6);
	adding_leak_memory_object(VM, 7);

	gc(VM);

	printf("\nSo. We end pushing elements to our universe. Now we will print them!!!\n");
	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	printf("+++++++++++++++++++++      PRINTING TIME      ++++++++++++++++++++++++\n");
	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
	
	//print = VM->stack[--VM->used_objects];
	print = VM->first;
	while (print != NULL) {
		object_print(print);
		if (print->type == pair_object) {
			while (print->head->type == pair_object || print->tail->type == pair_object)
			{
				if (print->head->type == pair_object) print = print->head;
				if (print->tail->type == pair_object) print = print->tail;

			}
		}
		print = print->next;
	}
	freeing(VM);
}

//void new_sequence_of_objects(VirtualMachine* vm)
//{
//	int random_num = random();
//	while (random_num > 0) {
//		Object* new_object;
//		if (random_num % 3 == 2) // if num mod 2 == 2, then we will push elemenst, else pair of pointers
//		{
//			if (vm->used_objects == vm->max_objects) gc(vm);
//			new_object = object_create(vm, int_object);
//			new_object->value = random_num; // setting certain int_value to our object
//			push__object__to_stack(vm, new_object);
//			set_first_pointer_of_LL_to_new_obj(vm, new_object); // setting --first pointer-- of linked list to --new object--
//			random_num /= 2;
//			printf("We add int_object with value: %d\n\n", new_object->value);
//		}
//		else { // pushing object with two pointers
//			if (vm->used_objects < 2) {
//				random_num = random();
//				continue;
//			}
//			else { // num of object in stack is more than 2, that mean that we can connect to object to one 
//				printf("Now we will connect two previous objects to one!\n");
//				new_object = object_create(vm, pair_object); // pair of two pointer: tail and head
//				new_object->tail = pop(vm);
//				printf("\nPrinting elements from tail pointer\n");
//				printing_elements(new_object->tail);
//				new_object->head = pop(vm);
//				printf("Printing elements from head pointer\n");
//				printing_elements(new_object->head);
//				printf("\n");
//				push__object__to_stack(vm, new_object);
//				set_first_pointer_of_LL_to_new_obj(vm, new_object);
//				random_num /= 2;
//			}
//		}
//	}
//}