/*
 * gpio_interface.cpp
 */

#include "gpio_interface.h"

extern uint32_t gpio_pin_def[3][2];

GPIO_Dev::GPIO_Dev(MCP23017_Dev * expander)
    : m_expander(expander)
{
    for (int i = 0;i < GPIO_MAX;i++)
        m_dir[i] = gpio_IN;
}

bool GPIO_Dev::init()
{
    return m_expander->init();
}

void GPIO_Dev::set_dir(t_gpio_pin pin, GPIO_dir dir)
{
    if (IS_PORT_A(pin))
        m_expander->set_dir(A, TO_PORT_A(pin), dir);
    else if (IS_PORT_B(pin))
        m_expander->set_dir(B, TO_PORT_B(pin), dir);
    else if (IS_GPIO_PIN(pin)) {
        uint32_t p;
        p = gpio_pin_def[TO_GPIO_PIN(pin)][dir];
        config_gpio(p);
    }
    m_state[(int)pin] = get_pin(pin);
    m_dir[(int)pin] = dir;
}


bool GPIO_Dev::get_pin(t_gpio_pin pin)
{
    if (IS_PORT_A(pin)) return m_expander->get_pin(A, TO_PORT_A(pin));
    else if (IS_PORT_B(pin)) return m_expander->get_pin(B, TO_PORT_B(pin));
    else if (IS_GPIO_PIN(pin)) {
        uint32_t p;
        p = gpio_pin_def[TO_GPIO_PIN(pin)][gpio_IN];
        return read_gpio(p);
    }
    else
        return false;
}

bool GPIO_Dev::event(t_gpio_pin pin)
{
    if (m_dir[(int)pin] == gpio_OUT) return false;

    // check event only on input pins
    if (IS_PORT_A(pin)) return m_expander->event(A, TO_PORT_A(pin));
    else if (IS_PORT_B(pin)) return m_expander->event(B, TO_PORT_B(pin));
    else {
        bool v = get_pin(pin);
        if (v != m_state[(int)pin]) {
            m_state[(int)pin] = v;
            return true;
        }
        else
            return false;
    }
}

void GPIO_Dev::set_pin(t_gpio_pin pin, bool val)
{
    if (IS_PORT_A(pin)) {
        m_expander->set_pin(A, TO_PORT_A(pin), val, true);
    }
    else if (IS_PORT_B(pin)) {
        m_expander->set_pin(B, TO_PORT_B(pin), val, true);
    }
    else if (IS_GPIO_PIN(pin)) {
        uint32_t p;
        p = gpio_pin_def[TO_GPIO_PIN(pin)][gpio_OUT];
        write_gpio(p, val);
    }
}

