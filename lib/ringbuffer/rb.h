/*
 * rbfactory.h
 *
 *  Created on: 07.07.2018
 *      Author: Andreas Kühne
 */

#ifndef RBFACTORY_H_
#define RBFACTORY_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

namespace pliskin_ringbuffer
{

class rb_base {
    protected:
        bool _full   = false;
        bool _empty  = true;
        bool _mem_ok = false;
        uint16_t _element_size;
        uint16_t _num_elements;
        uint16_t _read_ndx = 0;
        uint16_t _write_ndx = 0;
        uint8_t * _rb_data;
        uint16_t _peak_all_ndx = 0;
        bool _peak_start = false;

        void increment_rd_ndx (void);
        void increment_wr_ndx (void);

    public:
        rb_base (const uint16_t element_size, const uint16_t num_of_elements) : _element_size(element_size), _num_elements(num_of_elements) {
            this->_rb_data = (uint8_t *) calloc(num_of_elements, element_size);
            this->_mem_ok = (this->_rb_data != NULL);
        }
        ~rb_base () {
            if (this->_mem_ok)
            {
                this->_mem_ok = false;
                free(this->_rb_data);
            }
        }

        // no copy or move construction
        rb_base(const rb_base&) = delete;   
        rb_base(rb_base&&) = delete;
        rb_base& operator=(const rb_base&) = delete;
        rb_base& operator=(rb_base&&) = delete;

        uint16_t remaining_elm (void);    /**< returns the number of unread elements */
        bool peak (void * element);   /**< Returns the oldest element without removing it. Only valid if 'true' is returned */
        bool read (void * element);   /**< Returns the oldest element and removes it. Only valid if 'true' is returned */
        bool write (const void * element);  /**< Writes an element. Successful if 'true' is returned */
        bool write_lossy (const void * element); /**< Writes an element. If the buffer is full, the oldest element gets overwritten. Successful if 'true' is returned */
        void clear (void);  /**< Removes all elements */
        bool full (void) {return this->_full;};
        bool empty (void) {return this->_empty;};
        bool mem_ok (void) {return this->_mem_ok;};

        /** \brief Can be used with a for loop to peak all unread elements. 
         * 
         * \attention Only works if the content of the ring buffer is not changed during peaking!
         * 
         * \retval The number of unread elements */
        uint16_t peak_all_start (void) {this->_peak_all_ndx = this->_read_ndx; this->_peak_start = true; return this->remaining_elm();}; 

        /** \brief Returns the next element from peaking (started with \ref peak_all_start()). Only valid if 'true' is returned. */
        bool peak_all_next (void * element);
};

};
#endif /* RBFACTORY_H_ */
