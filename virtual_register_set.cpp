#include "virtual_register_set.h"
#include "virtual_register.h"
#include "serial_exc.h"

#include <wbmqtt/utils.h>

#include <cassert>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

TVirtualRegisterSet::TVirtualRegisterSet(const vector<PVirtualRegister> & virtualRegisters)
{
    VirtualRegisters.reserve(virtualRegisters.size());

    for (const auto & vreg: virtualRegisters) {
        VirtualRegisters.push_back(vreg);
    }
}

string TVirtualRegisterSet::GetTextValue() const
{
    ostringstream ss;
    bool first = true;

    for (const auto & vreg: VirtualRegisters) {
        if (!first) {
            ss << ";";
        }

        ss << vreg->GetTextValue();
        first = false;
    }

    return ss.str();
}

void TVirtualRegisterSet::SetTextValue(const string & value)
{
    const auto & textValues = StringSplit(value, ';');
    const auto valueCount = textValues.size();
    {
        const auto expectedValueCount = VirtualRegisters.size();

        if (expectedValueCount != valueCount) {
            throw TSerialDeviceException("value count mismatch for register set: expected '" +
                                            to_string(expectedValueCount) + "' actual '" +
                                            to_string(valueCount));
        }
    }

    for (size_t i = 0; i < valueCount; ++i) {
        const auto & vreg = VirtualRegisters[i];

        if (Global::Debug) {
            std::cerr << "setting device register: " << vreg->ToString() << " <- " <<
                textValues[i] << std::endl;
        }

        vreg->SetTextValue(textValues[i]);
    }
}

EErrorState TVirtualRegisterSet::GetErrorState() const
{
    EErrorState result = EErrorState::NoError;

    for (const auto & vreg: VirtualRegisters) {
        Add(result, vreg->GetErrorState());
    }

    return result;
}

bool TVirtualRegisterSet::GetValueIsRead() const
{
    return AllOf(VirtualRegisters, [](const PVirtualRegister & vreg){
        return vreg->GetValueIsRead();
    });
}

bool TVirtualRegisterSet::IsChanged(EPublishData data) const
{
    return AnyOf(VirtualRegisters, [data](const PVirtualRegister & vreg){
        return vreg->IsChanged(data);
    });
}

void TVirtualRegisterSet::ResetChanged(EPublishData data)
{
    for (const auto & vreg: VirtualRegisters) {
        vreg->ResetChanged(data);
    }
}