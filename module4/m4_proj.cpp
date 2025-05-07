/*
Briefing

A waiting area in a barbershop has very limited space.
There are three barbers, and two categories of customer: child and adult.
Barbers 1 and 2 cut only the hair of adults, while barber 3 cuts only
children’s hair.
There are 8 waiting spaces. Customers must come alone, except that children
must come with one parent; no one is allowed entry if there is no waiting space.
One haircut takes 12 minutes, with 1 minute gap before the next customer.
Once their hair is cut, customers leave the shop without re-entering the
waiting area; children’s parents however remain in the waiting area until
their child’s haircut is complete.
The clock on the wall ticks every minute.
In the case of a fire alarm, all customers must leave the shop in an orderly
fashion. 

Interrupts should be used for adult or child requesting entry, and for the fire
alarm. To ease testing, reduce each minute to one second, or a convenient
time-slot. Seat occupancy should be indicated by LEDs.
*/ 

#include "mbed.h"

// -- PIN ASSIGNMENTS --

// Buttons
#define ADULT_REQ p5
#define CHILD_REQ p6
#define FIRE p7

// LEDs
#define BARBER1 p8
#define BARBER2 p9
#define BARBER3 p10
#define WAITING1 p11
#define WAITING2 p12
#define WAITING3 p13
#define WAITING4 p14
#define WAITING5 p15
#define WAITING6 p16
#define WAITING7 p17
#define WAITING8 p18
#define NO_ENTRY p19
#define WALL_CLOCK p20

// for LED DigitalOuts
#if !defined OFF && !defined ON
#define OFF 0
#define ON 1
#endif

// --- CONFIGURATION CONSTANTS ---

// length of wall clock tick in seconds
#define TICK 1

// length of a haircut in ticks
#define HAIRCUT_TICKS 12
// ticks between haircuts -
// cannot be zero or the state machinie won't work.
#define BREAK_TICKS 1
_Static_assert(BREAK_TICKS > 0, "BREAK_TICKS cannot be zero");

#define WAITING_ROOM_CAPACITY 8


class WaitingRoom {
    // TODO This class should be a singleton unless inappropriate for embedded

    static DigitalOut leds[];

    public:

    volatile unsigned int adults;   // waiting for haircuts
    volatile unsigned int children;
    volatile unsigned int parents;  // waiting for children's haircuts
    
    WaitingRoom() : adults(0), children(0), parents(0)
    {}

    unsigned int occupancy() {
        return adults + children + parents;
    }

    bool canSeat(unsigned int n) {
        return (WAITING_ROOM_CAPACITY - occupancy() >= n);
    }
        
    // update LEDs to match ooccupanvy
    void update();

    // clear all seats
    void clear() {
        adults = 0;
        children = 0;
        parents = 0;
        update();
    }
};

DigitalOut WaitingRoom::leds[WAITING_ROOM_CAPACITY] = {
    DigitalOut (WAITING1, OFF),
    DigitalOut (WAITING2, OFF),
    DigitalOut (WAITING3, OFF),
    DigitalOut (WAITING4, OFF),
    DigitalOut (WAITING5, OFF),
    DigitalOut (WAITING6, OFF),
    DigitalOut (WAITING7, OFF),
    DigitalOut (WAITING8, OFF),
};
    
class Barber {
    public:
    enum CustomerType {ADULT, CHILD};
    
    private:
    enum {READY, BUSY, ON_BREAK} state;
    enum CustomerType customerType;
    unsigned int timer;
    DigitalOut led;

    public:

    Barber(PinName outputPin, CustomerType t)
       : state(READY)
       , customerType(t)
       , timer(0)
       , led(outputPin, OFF)
    {}

    bool isAvailable() { return state == READY; }

    void startHaircut() {
        state = BUSY;
        led = ON;
        timer = HAIRCUT_TICKS;
    }

    // state machine
    // main() is responsible for updating waitingRoom when all
    // synchronous processing is complete.
    void tick();

    void clear() {
        state = READY;
        led = OFF;
        timer = 0;
    }
};


// --- GLOBALS ---

InterruptIn adultReq(ADULT_REQ);
InterruptIn childReq(CHILD_REQ);
InterruptIn fireAlarm(FIRE);

DigitalOut noEntry(NO_ENTRY, OFF);
DigitalOut wallClock(WALL_CLOCK, OFF);

Barber barber1(BARBER1, Barber::ADULT);
Barber barber2(BARBER2, Barber::ADULT);
Barber barber3(BARBER3, Barber::CHILD);

WaitingRoom waitingRoom;


// --- FUNCTION DECLARATIONS ---

void adult_req_isr();
void child_req_isr();
void clear_all_seats();
void tick();


// --- MAIN ---

int main() {
    // initialize interrupts
    adultReq.rise(adult_req_isr);
    childReq.rise(child_req_isr);
    fireAlarm.rise(clear_all_seats);

    while (1) {
        // blink wall clock LED and do synchronous processing
        wallClock = ON;
        tick();
        wait(0.2);
        wallClock = OFF;
        wait(TICK - 0.2);
    }
    return 0;
}


void adult_req_isr() {
    noEntry = OFF;
    if (barber1.isAvailable()) { 
        barber1.startHaircut();
    }
    else if (barber2.isAvailable()) { 
        barber2.startHaircut();
    }
    else if (waitingRoom.canSeat(1)) {
        waitingRoom.adults++;
        waitingRoom.update();
    }
    else {
        noEntry = ON;
    }
}

// child requesting entry with 1 parent
void child_req_isr() {
    noEntry = OFF;
    if (barber3.isAvailable() && waitingRoom.canSeat(1)) {
        barber3.startHaircut();
        waitingRoom.parents++;
        waitingRoom.update();
    }
    else if (waitingRoom.canSeat(2)) {
        waitingRoom.children++;
        waitingRoom.parents++;
        waitingRoom.update();
    }
    else {
        noEntry = ON;
    }
}

// If fire alarm goes off, empty the building!
// (Used as ISR)
void clear_all_seats() {
    barber1.clear();
    barber2.clear();
    barber3.clear();
    waitingRoom.clear();
    noEntry = OFF;
}


// Synchronous Processing

void tick() {
    barber1.tick();
    barber2.tick();
    barber3.tick();
    waitingRoom.update();
}

void Barber::tick() {
    if (timer > 0) {
        if (--timer == 0) {
            if (state == BUSY) {
                // haircut complete
                led = OFF;
                if (customerType == CHILD) {
                    waitingRoom.parents--;
                }
                state = ON_BREAK;
                timer = BREAK_TICKS;
            }
            else if (state == ON_BREAK) {
                state = READY;
            }
        }
    }

    if (state == READY) {
        // check for new customer
        if (customerType == ADULT && waitingRoom.adults != 0) {
            waitingRoom.adults--;
            noEntry = OFF;
            startHaircut();
        }
        else if (customerType == CHILD && waitingRoom.children != 0) {
            waitingRoom.children--;
            noEntry = OFF;
            startHaircut();
        }
    }
}

void WaitingRoom::update() {
    for (int i=0; i < occupancy(); i++) {
        leds[i] = ON;
    }
    for (int i=occupancy(); i < WAITING_ROOM_CAPACITY; i++) {
        leds[i] = OFF;
    }
}

