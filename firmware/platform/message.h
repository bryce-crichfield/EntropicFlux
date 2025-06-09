#pragma once

#include <stdlib.h>
#include <stdatomic.h>

typedef struct {

} messge_t;

typedef struct {
    message_t *messages;
    size_t    size;
    atomic_size_t write_index;
    atomic_size_t read_index;
} message_buffer_t;

void message_buffer_init(message_buffer_t *buffer, size_t size) {
    buffer->messages = (message_t *)malloc(size * sizeof(message_t));
    buffer->size = size;
    atomic_init(&buffer->write_index, 0);
    atomic_init(&buffer->read_index, 0);
}

void message_buffer_write(message_buffer_t *buffer, message_t *message) {
    size_t write_index = atomic_load(&buffer->write_index);
    size_t next_index = (write_index + 1) % buffer->size;
    if(next_index != atomic_load(&buffer->read_index)) {
        buffer->messages[write_index] = *message;
        atomic_store(&buffer->write_index, next_index);
    }
}

bool message_buffer_read(message_buffer_t *buffer, message_t *message) {
    size_t read_index = atomic_load(&buffer->read_index);
    if(read_index == atomic_load(&buffer->write_index)) {
        return false;
    }
    *message = buffer->messages[read_index];
    atomic_store(&buffer->read_index, (read_index + 1) % buffer->size);
    return true;
}