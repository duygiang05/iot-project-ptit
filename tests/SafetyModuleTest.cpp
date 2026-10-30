#include <cassert>
#include <iostream>

#include "SafetyModule.h"

static void expectEvent(SafetyModule& module, SafetyEventType expected) {
  SafetyEvent event;
  assert(module.popEvent(event));
  assert(event.type == expected);
}

static void testHeldButtonEmitsOneSos() {
  SafetyModule module;
  module.begin(false, 0);
  module.update(true, 1.0f, 0.0f, 100);
  module.update(true, 1.0f, 0.0f, 135); // debounce press
  module.update(true, 1.0f, 0.0f, 3134); // 2999 ms held
  SafetyEvent event;
  assert(!module.popEvent(event));
  module.update(true, 1.0f, 0.0f, 3135); // 3000 ms held
  expectEvent(module, SafetyEventType::SOS);
  module.update(true, 1.0f, 0.0f, 5000);
  assert(!module.popEvent(event)); // no repeated SOS for one hold
}

static void testImpactTiltAndCancel() {
  SafetyModule module;
  module.begin(false, 0);
  module.update(false, 2.0f, 10.0f, 50); // brief impact
  module.update(false, 1.0f, 70.0f, 100); // cane remains tilted
  module.update(false, 1.0f, 70.0f, 699);
  SafetyEvent event;
  assert(!module.popEvent(event));
  module.update(false, 1.0f, 70.0f, 700);
  expectEvent(module, SafetyEventType::CANE_FALL_SUSPECTED);

  module.update(true, 1.0f, 70.0f, 800);
  module.update(true, 1.0f, 70.0f, 835); // press cancels countdown
  expectEvent(module, SafetyEventType::CANCEL);
  module.update(false, 1.0f, 70.0f, 1000);
  module.update(false, 1.0f, 70.0f, 1700);
  assert(!module.popEvent(event)); // cancellation clears the latched impact
}

static void testUncancelledFallAlerts() {
  SafetyModule module;
  module.begin(false, 0);
  module.update(false, 2.0f, 10.0f, 50);
  module.update(false, 1.0f, 70.0f, 100);
  module.update(false, 1.0f, 70.0f, 700);
  expectEvent(module, SafetyEventType::CANE_FALL_SUSPECTED);
  module.update(false, 1.0f, 70.0f, 10700);
  expectEvent(module, SafetyEventType::CANE_FALL_ALERT);
  assert(module.buzzerOn());
}

int main() {
  testHeldButtonEmitsOneSos();
  testImpactTiltAndCancel();
  testUncancelledFallAlerts();
  std::cout << "SafetyModule scenarios passed\n";
}
