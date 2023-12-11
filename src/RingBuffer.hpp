#include <stdint.h>
#include <stdlib.h>
#include <memory>
#include <mutex>
#include <optional>
#include <cstring>


template <class T>
class RingBuffer
{
private:
    std::mutex mutex_;
    std::unique_ptr<T[]> buf_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t count_ = 0;
    const size_t max_size_;
    // bool full_ = false;

public:
    explicit RingBuffer(size_t size):max_size_(size),buf_(std::unique_ptr<T[]>(new T[size]))
    {    };

   void reset()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        head_ = tail_;
        count_ = 0;
    }

    bool empty() const
    {
        //if head and tail are equal, we are empty
        return (count_ == 0);
    }

    bool full() const
    {
        //if head and tail are equal, we are empty
        return (count_ >= max_size_);
    }

    size_t capacity() const
    {
        return max_size_;
    }

    size_t size() const
    {
        // size_t size = max_size_;

        // if(!full_)
        // {
        //     if(head_ >= tail_)
        //     {
        //         size = head_ - tail_;
        //     }
        //     else
        //     {
        //         size = max_size_ + head_ - tail_;
        //     }
        // }

        return count_;
    }

    void put(T item)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        buf_[head_] = item;

        if(full())
        {
            tail_ = (tail_ + 1) % max_size_;
        }else{
            count_++;
        }

        head_ = (head_ + 1) % max_size_;
        // full_ = head_ == tail_;
    }

    /** 
     * @brief  Adds array of data to the ring buffer.
     * @param data is the array containing the data to be added
     * @param length is the length of the array
     * Buffer must be twice the length of the data to prevent double wrap arounds
     */
    int putMany(T data[], int length)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        int slots_to_end = max_size_ - head_; //buffer capacity minus current number of values
        int slots_to_write = length >= slots_to_end ? slots_to_end : length; 
        
        if(length > max_size_) return -1;
        memcpy(&buf_[head_], data, sizeof(data[0])*(slots_to_write)); //Fill slots to end of array with data.
        count_ += slots_to_write;
        if(count_ > max_size_){
        // if ((tail_ >= head_) && (tail_ < (head_ + slots_to_write)))
        // {
            tail_ = (head_ + slots_to_write) % max_size_;
            count_ = max_size_;
        }
        
        head_ = (head_ + slots_to_write) % max_size_;
        
        if ((length - slots_to_end) > 0)
        {
            slots_to_write = length - slots_to_end;
            memcpy(&buf_[head_], &data[slots_to_end], sizeof(data[0])*(slots_to_write));
            count_ += slots_to_write;
            if(count_ > max_size_){
            // if ((tail_ >= head_) && (tail_ < (head_ + slots_to_write)))
            // {
                tail_ = (head_ + slots_to_write) % max_size_;
                count_ = max_size_;
            }
            
            head_ = (head_ + slots_to_write) % max_size_;
        }
        return 0;
    }

    void printBuff() {
        printf("Head: %ld, Tail: %ld\r\n[", head_, tail_);
        for(int i = 0; i < max_size_; i++){
            printf("%d ", buf_[i]);
        }
        printf("]\r\n");
    }

    int getMany(T *data, int length)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if(empty())
        {
            return 0;
        }
        int itemsRetrieved = 0;
        int slots_to_end = tail_ >= head_ ? max_size_ - tail_ : size(); //buffer capacity minus current number of values
        int slots_to_read = length >= slots_to_end ? slots_to_end : length; 
        memcpy(data,&buf_[tail_],sizeof(T) * slots_to_read);
        tail_ = (tail_ + slots_to_read) % max_size_;
        itemsRetrieved = slots_to_read;
        count_ -= slots_to_read;

        if (tail_ >= head_)
        {
            return itemsRetrieved;
        }

        slots_to_end = tail_ >= head_ ? max_size_ - tail_ : size();
        slots_to_read = (length - itemsRetrieved) >= slots_to_end ? slots_to_end : length - itemsRetrieved;
        memcpy(&data[itemsRetrieved],&buf_[tail_],sizeof(T) * slots_to_read);
        tail_ = (tail_ + slots_to_read) % max_size_;
        itemsRetrieved += slots_to_read;
        count_ -= slots_to_read;
        return itemsRetrieved;
    }

    std::optional<T> get()
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if(empty())
        {
            return std::nullopt;
        }

        //Read data and advance the tail (we now have a free space)
        auto val = buf_[tail_];
        count_--;
        tail_ = (tail_ + 1) % max_size_;

        return val;
    }
};