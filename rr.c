#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <math.h>

#include "lwp.h"

thread head = NULL;
thread curr = NULL;

/* sched_one = next */
/* sched_two = prev */

void init(void);

void shutdown(void);

void admit(thread new)
{
    thread temp = head;
    if (!head)
    {
        head = new;
        new->sched_one = NULL;
        new->sched_two = NULL;
    }
    else
    {
        while (temp->sched_one)
        {
            temp = temp->sched_one;
        }
        temp->sched_one = new;
        new->sched_two = temp;
        new->sched_one = NULL;
    }
}

void remove(thread victim)
{
    if (!head || !victim)
    {
        return;
    }

    if (curr == victim)
    {
        curr = victim->sched_one;
    }
    if (head == victim)
    {
        head = head->sched_one;
        if (head)
        {
            head->sched_two = NULL;
        }
    }
    else
    {
        if (victim->sched_two)
        {
            victim->sched_two->sched_one = victim->sched_one;
        }
        if (victim->sched_one)
        {
            victim->sched_one->sched_two = victim->sched_two;
        }
    }
}

thread next(void)
{
    if (!head)
    {
        return NULL;
    }
    if (!curr)
    {
        curr = head;
    }
    else
    {
        curr = curr->sched_one;
        if (!curr)
        {
            curr = head; // wrap around to beginning
        }
    }
    return curr;
}

int qlen()
{
    int len = 0;
    if (!head)
    {
        return 0;
    }
    thread temp = head;
    while (temp)
    {
        len++;
        temp = temp->sched_one;
    }
    return len;
}

struct scheduler RR = {NULL, NULL, admit, remove, next, qlen};
