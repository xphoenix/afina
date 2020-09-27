#include <iostream>
#include <memory>

struct node{
	
	int data;
	std::unique_ptr<node> next;
	node* prev;
};

class list{

	size_t size;
	std::unique_ptr<node> head;
	node* tail;

public:
	list(){

		head = nullptr;
		tail = nullptr;
		size = 0;
	}

	void push( int value ){

		if( size ){

			
			head = std::make_unique<node>(node{value, std::move(head), nullptr});
			head->next->prev = head.get();
			
			// tail->next = std::make_unique<node>(node{value, nullptr, tail});
			// tail = tail->next.get();
		}
		else{
			head = std::make_unique<node>(node{value, nullptr, nullptr});
			tail = head.get();
		}
		
		++size;
	}

	void pop(){

		if( size > 1 ){
		

			tail = tail->prev;
			tail->next = nullptr;
		}
		else
		if( size == 1 ){

			head = nullptr;
			tail = nullptr;
		}
		else
			throw std::runtime_error("pop(): No elements!");

		--size;
	}

	void print(){

		node* cur_node = head.get();

		while( cur_node != nullptr ){
			
			std::cout << cur_node->data << " ";
			cur_node = cur_node->next.get();
		}

		std::cout << std::endl;
	}
};

int main(){

	list lst;

	lst.push(0);
	lst.push(1);
	lst.pop();
	lst.pop();
//	lst.pop();
	lst.push(3);
	lst.push(4);

	lst.print();

	return 0;
}
