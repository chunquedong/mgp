/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#ifndef MOUSE_H_
#define MOUSE_H_

#include "base/Base.h"

namespace mgp
{

class MotionEvent {
public:
  enum MotionType {
    press,
    release,
    touchMove,
    mouseMove,
    longPress,
    click,
    cancel,
    wheel,
    other,
  };
    
    enum Button {
      left,
      middle,
      right,
    };
    
  /**
  * Maximum simultaneous touch points supported.
  */
  static const unsigned int MAX_TOUCH_POINTS = 10;

  /**
  * event occurs time
  */
  int64_t time;

  /**
  * event type
  */
  MotionType type;

  /**
  * The order of occurrence for multiple touch contacts starting at zero.
  */
  int contactIndex;

  /**
  ** X coordinates
  **/
  int x;

  /**
  ** Y coordinates
  **/
  int y;

  /**
  ** Delta value of event.  For mouse wheel events this is the
  ** amount the mouse wheel has traveled.
  **/
  int wheelDelta;

  /**
  ** Number of mouse clicks.
  **/
  int count;

  /**
  ** Mouse button number pressed
  **/
  Button button;

  /**
  ** Current pressure of pointer
  **/
  double pressure;

  /**
  ** Current size of pointer
  **/
  double size;

  /**
  ** native event
  **/
  void *rawEvent;

  /**
  ** For muilt touch event
  **/
  std::vector<MotionEvent> pointers;

public:
  MotionEvent() : contactIndex(0), time(0),
    type(other),
    wheelDelta(0),
    count(0),
    button(left),
    pressure(0),
    size(0),
    rawEvent(NULL)
    {
    }
};


typedef MotionEvent& Mouse;


}

#endif
