/*
    This file is part of Kismet

    Kismet is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kismet is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Kismet; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifndef __PHY_UAV_DRONE_H__
#define __PHY_UAV_DRONE_H__

#include "trackedelement.h"
#include "tracked_location.h"

/* An abstract model of a UAV (drone/quadcopter/plane) device.
 *
 * A UAV may have multiple communications protocols (bluetooth, wi-fi, RF)
 * which comprise the same device; references to the independent devices of
 * other phys are linked in the optional descriptors.
 *
 * A UAV is worthy of its own top-level device because it also introduces
 * independent tracking requirements; location history may be sources
 * from UAV telemetry independent of the devices/kismet sensor locations.
 *
 */

class uav_tracked_telemetry : public tracker_component {
public:
    uav_tracked_telemetry(GlobalRegistry *in_globalreg, int in_id) :
        tracker_component(in_globalreg, in_id) {

        register_fields();
        reserve_fields(NULL);
    }

    uav_tracked_telemetry(GlobalRegistry *in_globalreg, int in_id,
            SharedTrackerElement e) :
        tracker_component(in_globalreg, in_id) {

        register_fields();
        reserve_fields(e);
    }

    virtual ~uav_tracked_telemetry() { }

    __ProxyTrackable(location, kis_tracked_location_triplet, location);
    __Proxy(yaw, double, double, double, yaw);
    __Proxy(pitch, double, double, double, pitch);
    __Proxy(roll, double, double, double, roll);
    __Proxy(height, double, double, double, height);
    __Proxy(v_north, double, double, double, v_north);
    __Proxy(v_east, double, double, double, v_east);
    __Proxy(v_up, double, double, double, v_up);

    __Proxy(motor_on, uint8_t, bool, bool, motor_on);
    __Proxy(airborne, uint8_t, bool, bool, airborne);

protected:
    virtual void register_fields() {
        tracker_component::register_fields();

        RegisterComplexField("kismet.uav.telemetry.location",
                std::shared_ptr<kis_tracked_location_triplet>(new kis_tracked_location_triplet(globalreg, 0)),
                "GPS location");

        RegisterField("kismet.uav.telemetry.yaw", TrackerDouble, 
                "yaw", &yaw);
        RegisterField("kismet.uav.telemetry.pitch", TrackerDouble, 
                "pitch", &pitch);
        RegisterField("kismet.uav.telemetry.roll", TrackerDouble,
                "roll", &roll);

        RegisterField("kismet.uav.telemetry.height", TrackerDouble,
                "height above ground", &height);
        RegisterField("kismet.uav.telemetry.v_north", TrackerDouble,
                "velocity relative to n/s", &v_north);
        RegisterField("kismet.uav.telemetry.v_east", TrackerDouble,
                "velocity relative to e/w", &v_east);
        RegisterField("kismet.uav.telemetry.v_up", TrackerDouble,
                "velocity relative to up/down", &v_up);

        RegisterField("kismet.uav.telemetry.motor_on", TrackerUInt8,
                "device reports motor enabled", &motor_on);
        RegisterField("kismet.uav.telemetry.airborne", TrackerUInt8,
                "device reports UAV is airborne", &airborne);

    }

    std::shared_ptr<kis_tracked_location_triplet> location;

    SharedTrackerElement yaw;
    SharedTrackerElement pitch;
    SharedTrackerElement roll;

    SharedTrackerElement height;
    SharedTrackerElement v_north;
    SharedTrackerElement v_east;
    SharedTrackerElement v_up;

    SharedTrackerElement motor_on;
    SharedTrackerElement airborne;

};

class uav_tracked_device : public tracker_component {
public:
    uav_tracked_device(GlobalRegistry *in_globalreg, int in_id) :
        tracker_component(in_globalreg, in_id) {

        register_fields();
        reserve_fields(NULL);
    }

    uav_tracked_device(GlobalRegistry *in_globalreg, int in_id,
            SharedTrackerElement e) :
        tracker_component(in_globalreg, in_id) {

        register_fields();
        reserve_fields(e);
    }

    virtual ~uav_tracked_device() { }


protected:
    virtual void register_fields() {
        tracker_component::register_fields();

    }

    SharedTrackerElement uav_manufacturer;
    
    SharedTrackerElement uav_serialnumber;

};

#endif

