/**
 ******************************************************************************
 * @author: GaoKong
 * @date:   24/07/2024
 ******************************************************************************
**/

#include "ring_buffer.h"

void ring_buffer_char_init(ring_buffer_char_t* ring_buffer, void* buffer_contain, uint16_t buffer_size) {
    /* default index */
	ring_buffer->tail_index = 0;
	ring_buffer->head_index = 0;
	ring_buffer->fill_size = 0;

    /* assign the buffers */
	ring_buffer->buffer_size = buffer_size;
	ring_buffer->buffer = buffer_contain;
}

uint16_t ring_buffer_char_availble(ring_buffer_char_t* ring_buffer) {
	return ring_buffer->fill_size;
}

bool ring_buffer_char_is_empty(ring_buffer_char_t* ring_buffer) {
	return (ring_buffer->fill_size == 0) ? true : false;
}

bool ring_buffer_char_is_full(ring_buffer_char_t* ring_buffer) {
	return (ring_buffer->fill_size == ring_buffer->buffer_size) ? true : false;
}

void ring_buffer_char_put(ring_buffer_char_t* ring_buffer, uint8_t c) {
	uint16_t next_tail_index;
	uint16_t next_head_index;

    /* put char to tail array */
	ring_buffer->buffer[ring_buffer->tail_index] = c;

    /* get next tail index after put char */
	next_tail_index = (++ring_buffer->tail_index) % ring_buffer->buffer_size;
	ring_buffer->tail_index = next_tail_index;

	if (ring_buffer->fill_size == ring_buffer->buffer_size) {
		next_head_index = (++ring_buffer->head_index) % ring_buffer->buffer_size;
		ring_buffer->head_index = next_head_index;
	}
	else {
		ring_buffer->fill_size++;
	}
}

uint8_t	ring_buffer_char_get(ring_buffer_char_t* ring_buffer) {
	uint16_t ret = 0;
	uint16_t next_head_index;

	if (ring_buffer->fill_size) {
		ret = ring_buffer->buffer[ring_buffer->head_index];

        /* get next head index after get char*/
		next_head_index = (++ring_buffer->head_index) % ring_buffer->buffer_size;
		ring_buffer->head_index = next_head_index;

		ring_buffer->fill_size--;
	}

	return ret;
}
