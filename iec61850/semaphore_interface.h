#pragma once

#include <stdint.h>

// Abstract semaphore type
typedef struct semaphore_t semaphore_t;

// Semaphore creation function
semaphore_t* semaphore_create(void);

// Take semaphore (block until available)
void semaphore_take(semaphore_t* sem);

// Release semaphore
void semaphore_release(semaphore_t* sem);

// Destroy semaphore
void semaphore_destroy(semaphore_t* sem);