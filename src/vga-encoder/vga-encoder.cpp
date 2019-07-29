#include "vga-encoder.hpp"

/* Control stage */
void
vga_encoder::thread_process()
{
    long int iterations = 0;
    while(iterations < MAX_ITERATIONS) {
        wait(next_state_t);
        state = next_state;
        FSM_next_state();
        FSM_output_logic();
        iterations++;
    }
}

void
vga_encoder::FSM_next_state()
{
    switch (state) {
    case FSM_VSYNC:
        next_state = FSM_V_BACK_PORCH;
        break;
    case FSM_V_BACK_PORCH:
        next_state = FSM_H_SYNC;
        break;
    case FSM_H_SYNC:
        next_state = FSM_H_BACK_PORCH;
        break;
    case FSM_H_BACK_PORCH:
        next_state = FSM_SEND_PIXELS;
        break;
    case FSM_SEND_PIXELS:
        if (col == COLS - 2) {
            next_state = FSM_H_FRONT_PORCH;
        } else {
            next_state = FSM_SEND_PIXELS;
        }
        break;
    case FSM_H_FRONT_PORCH:
        if (row == ROWS - 1) {
            next_state = FSM_V_FRONT_PORCH;
        } else {
            next_state = FSM_H_SYNC;
        }
        break;
    case FSM_V_FRONT_PORCH:
        next_state = FSM_VSYNC;
        break;
    default:
        next_state = FSM_VSYNC;
        break;
    }
}

void
vga_encoder::FSM_output_logic()
{
    switch (state) {
    case FSM_VSYNC:
        v_sync.write(0);
        h_sync.write(1);
        col = 0;
        row = 0;
        next_state_t.notify(DELAY_VSYNC, SC_NS);
        break;
    case FSM_V_BACK_PORCH:
        v_sync.write(1);
        next_state_t.notify(DELAY_V_BACK_PORCH, SC_NS);
        break;
    case FSM_V_FRONT_PORCH:
        v_sync.write(1);
        next_state_t.notify(DELAY_V_FRONT_PORCH, SC_NS);
        break;
    case FSM_H_SYNC:
        h_sync.write(0);
        col = 0;
        next_state_t.notify(DELAY_H_SYNC, SC_NS);
        break;
    case FSM_H_BACK_PORCH:
        h_sync.write(1);
        next_state_t.notify(DELAY_H_BACK_PORCH, SC_NS);
        break;
    case FSM_H_FRONT_PORCH:
        h_sync.write(1);
        row++;
        next_state_t.notify(DELAY_H_FRONT_PORCH, SC_NS);
        break;
    case FSM_SEND_PIXELS:
        h_sync.write(1);
        send_pixel();
        col++;
        next_state_t.notify(DELAY_SEND_PIXELS, SC_PS);
        break;
    default:
        h_sync.write(1);
        v_sync.write(1);
        next_state_t.notify(DELAY_DEFAULT, SC_NS);
        break;
    }
}

/* Input ports */
void
vga_encoder::reset()
{
    col = 0;
    row = 0;
    next_state = FSM_VSYNC;
    next_state_t.notify(READ_DELAY, SC_NS);
}

/* Datapath */
void
vga_encoder::send_pixel()
{
    /* Enqueue next pixel */
    if(pixels_queue.empty()) {
        pixel_out = 0;
    } else {
        pixel_out = pixels_queue.front();
        pixels_queue.pop();
    }
    
    /* Write to output */
    put_rgb_signal();
}

/* TLM implementation */
void
vga_encoder::put_rgb_signal()
{
    initiator->write(DAC_ADDRESS, (int)pixel_out, tlm::TLM_WRITE_COMMAND);
}

/* Node inheritation */
void
vga_encoder::reading_process()
{
    while(true) {
        wait(*(incoming_notification));
        bool command = target->command;
        unsigned short data = target->incoming_buffer;
        wait(sc_time(BUS_DELAY, SC_NS));

        if(command == tlm::TLM_WRITE_COMMAND) {
            /* Copy pixels to internal buffer */
            unsigned short pixel_in = data & 0xFFF;

            /* Write pixels into queue */
            pixels_queue.push(pixel_in);
        }
    }
}
