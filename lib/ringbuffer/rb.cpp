#include "rb.h"

namespace pliskin_ringbuffer {

    uint16_t rb_base::remaining_elm (void)
    {
        uint16_t remaining_elements = 0;

        if (!this->empty())
        {
            remaining_elements = this->_num_elements;

            if (!this->full())
            {
                if (this->_write_ndx >= this->_read_ndx)
                    remaining_elements = this->_write_ndx - this->_read_ndx;
                else
                {
                    remaining_elements -= this->_read_ndx;
                    remaining_elements += this->_write_ndx;
                }
            }
                
        }

        return remaining_elements;
    }

    bool rb_base::peak (void * element)
    {
        if (this->empty() || !this->_mem_ok)
            return false;
        
        memcpy(element, this->_rb_data + (this->_element_size * this->_read_ndx), this->_element_size);
        return true;
    }

    bool rb_base::read (void * element)
    {
        bool peak_ok = this->peak(element);

        if (peak_ok)
            this->increment_rd_ndx();

        return peak_ok;
    }

    void rb_base::increment_rd_ndx (void)
    {
        this->_read_ndx++;
        if (this->_read_ndx >= this->_num_elements)
            this->_read_ndx = 0;

        this->_full = false;
        this->_empty |= (this->_read_ndx == this->_write_ndx);
    }

    void rb_base::increment_wr_ndx (void)
    {
        this->_write_ndx++;
        this->_empty = false;

        if(this->_write_ndx >= this->_num_elements)
            this->_write_ndx = 0;

        if (this->_write_ndx == this->_read_ndx)
            this->_full = true;
    }

    bool rb_base::write (const void * element)
    {
        if (this->full() || !this->_mem_ok)
            return false;

        memcpy(this->_rb_data + (this->_element_size * this->_write_ndx), element, this->_element_size);
        this->increment_wr_ndx();
        return true;
    }

    bool rb_base::write_lossy (const void * element)
    {
        if (this->full())
            this->increment_rd_ndx();

        return this->write(element);
    }

    void rb_base::clear (void)
    {
        this->_read_ndx = this->_write_ndx = 0;
        this->_full = false;
        this->_empty = true;

        return;
    }

    bool rb_base::peak_all_next (void * element)
    {
        if (!this->_mem_ok || this->empty() || (!this->_peak_start && (this->_peak_all_ndx == this->_write_ndx)))
            return false;

        memcpy(element, this->_rb_data + (this->_element_size * this->_peak_all_ndx), this->_element_size);
        this->_peak_start = false;
        this->_peak_all_ndx++;
        if (this->_peak_all_ndx >= this->_num_elements)
            this->_peak_all_ndx = 0;

        return true;
    }
};