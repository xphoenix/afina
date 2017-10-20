#include <afina/allocator/Simple.h>

#include <afina/allocator/Pointer.h>
#include <afina/allocator/Error.h>

namespace Afina {
    namespace Allocator {
        
        
        Simple::Simple(void *base, size_t base_size): _base(base),_base_len(base_size) {
            //init
            buffer.first = base;
            buffer.last = static_cast<char *>(base) + base_size - sizeof(block) ;
            root = static_cast<block *>(buffer.last);
            root->prev = nullptr;
            root->next  = nullptr;
            root->first = nullptr;
            root->last  = nullptr;
            last_elmnt_lst = root;
            free_space = nullptr;
        }

/**
 * TODO: semantics
 * @param N size_t
 */
Pointer Simple::alloc_empty(size_t N) {
    root->first = buffer.first;
    root->last = static_cast<void *>(static_cast<char*>(buffer.first) + N);
    return Pointer(root);
    
}
        
Pointer Simple::alloc_in_fspace(size_t N){
    if (free_space != nullptr) {
        block *new_free_point = free_space->prev;
        free_space->prev = root;
        free_space->first = root->last;
        root->next = free_space;
        free_space->last = static_cast<void *>(static_cast<char *>(root->last) + N);
        root = free_space;
        free_space = new_free_point;
        root->next = nullptr;
    }
    else{
        last_elmnt_lst--;
        root->next = last_elmnt_lst;
        last_elmnt_lst->prev = root;
        last_elmnt_lst->first = root->last;
        last_elmnt_lst->last = static_cast<void*>(static_cast<char*>(root->last) + N);
        last_elmnt_lst->next = nullptr;
        root = last_elmnt_lst;
    }
    return Pointer(root);
}
        
Pointer Simple::find_hole_space(size_t N){
    
    for(block *cur = static_cast<block *>(buffer.last); cur->next != nullptr; cur = cur->next){
        if((cur->next)->first != nullptr){
            size_t sz = static_cast<char*>((cur->next)->first) - static_cast<char*>(cur->last);
            if(sz >= N){
                if (free_space != nullptr){
                    block *new_free_points = free_space->prev;
                    free_space->next = cur->next;
                    free_space->prev = cur;
                    free_space->first = cur->last;
                    free_space->last = static_cast<void*>(static_cast<char*>(cur->last) + N);
                    
                    (cur->next)->prev = free_space;
                    block *new_one = free_space;
                    free_space = new_free_points;
                    return Pointer(new_one);
                }
                else{
                    last_elmnt_lst--;
                    last_elmnt_lst->prev = cur;
                    last_elmnt_lst->next = cur->next;
                    last_elmnt_lst->first = cur->last;
                    last_elmnt_lst->last = static_cast<void*>(static_cast<char*>(cur->last) + N);
                    (cur->next)->prev = last_elmnt_lst;
                    cur->next = last_elmnt_lst;
                    return Pointer(last_elmnt_lst);
                }
            }
        }
    }
    throw AllocError(AllocErrorType::NoMemory, "Try defraq");
}
        
/**
 * TODO: semantics
 * @param p Pointer
 * @param N size_t
 */
Pointer Simple::alloc(size_t N) {
    if (root->prev  == nullptr &&
        root->next  == nullptr &&
        root->first == nullptr &&
        root->last  == nullptr &&
        N + sizeof(block) < static_cast<char*>(buffer.last) - static_cast<char*>(buffer.first))
        return alloc_empty(N);
    
    if (last_elmnt_lst > static_cast<void*>(static_cast<char*>(last_elmnt_lst->last) + N + sizeof(block)))
        return alloc_in_fspace(N);
    
    return find_hole_space(N);
}
        
/**
 * TODO: semantics
 * @param p Pointer
 * @param N size_t
 */
void Simple::realloc(Pointer &p, size_t N) {
    void *info_link = p.get();
    size_t size = p.get_size();
    free(p);
    p = alloc(N);
    memcpy(p.get(), info_link, size);
}
/**
 * TODO: semantics
 * @param p Pointer
 */
void Simple::defrag() {
    for(block *cur = static_cast<block *>(buffer.last)->next; cur != nullptr; cur = cur->next){
        if(cur->first != nullptr) {
            if (cur->first > cur->prev->last) {
                size_t sz = static_cast<char*>(cur->last) - static_cast<char*>(cur->first);
                memcpy(cur->prev->last, cur->first, sz);
                cur->first = cur->prev->last;
                cur->last = static_cast<char*>(cur->first) + sz;
            }
        }
        else{
            cur->last = cur->prev->last;
        }
    }
}
        
void Simple::free(Pointer& p){
    block *prev = nullptr;
    block *get = p.get_ptr();
    
    if (get != nullptr){
        if (free_space == nullptr){
            free_space = get;
            prev = nullptr;
        }
        else{
            free_space->next = get;
            prev = free_space;
            free_space = free_space->next;
        }

        if (free_space->next != nullptr){
            if (free_space->prev != nullptr){
                block *next = free_space->next;
                block *last = free_space->prev;
                next->prev = last;
                last->next = next;
                
            }
            else{
                (free_space->next)->prev = nullptr;
            }
        }
        else{
            if (free_space->prev != nullptr){
                root = root->prev;
                root->next = nullptr;
            }
            else{
                root->next = nullptr;
                root->last = nullptr;
                root->first = nullptr;
                root->last = nullptr;
            }
        }
        if (root != free_space){
            free_space->first = nullptr;
            free_space->last = nullptr;
            free_space->prev = prev;
            free_space->next = nullptr;
        }
        else{
            free_space = prev;
            if (free_space != nullptr)
                free_space->next = nullptr;
        }
    }
    
    p = nullptr;
}


    } // namespace Allocator
} // namespace Afina
