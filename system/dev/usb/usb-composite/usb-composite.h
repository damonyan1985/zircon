// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <ddk/binding.h>
#include <ddk/device.h>
#include <ddk/protocol/usb.h>
#include <lib/sync/completion.h>
#include <zircon/hw/usb.h>

#include <threads.h>
#include <stdatomic.h>

typedef enum {
    // The interface has not been claimed and no device has been created for it.
    AVAILABLE,
    // Another interface has claimed the interface.
    CLAIMED,
    // A child device has been created for the interface.
    CHILD_DEVICE
} interface_status_t;

typedef struct usb_bus usb_bus_t;

// Represents a USB top-level device
typedef struct usb_device {
    zx_device_t* zxdev;
    usb_protocol_t usb;

    // ID assigned by host controller
    uint32_t device_id;

    usb_device_descriptor_t device_desc;
    usb_configuration_descriptor_t** config_descs;
    int current_config_index;

    atomic_bool langids_fetched;
    atomic_uintptr_t lang_ids;

    mtx_t interface_mutex;
    // Array storing whether interfaces from 0 to bNumInterfaces-1
    // are available, claimed or is a child device.
    interface_status_t* interface_statuses;
    // list of child devices (for USB composite devices)
    list_node_t children;
} usb_device_t;

zx_status_t usb_device_add(usb_bus_t* bus, uint32_t device_id, uint32_t hub_id,
                           usb_speed_t speed, usb_device_t** out_device);

void usb_device_remove(usb_device_t* dev);

// Marks the interface as claimed, removing the device if it exists.
// Returns an error if the interface was already claimed by another interface.
zx_status_t usb_device_claim_interface(usb_device_t* dev, uint8_t interface_id);

// device protocol functions shared with usb_interface_t
zx_status_t usb_device_control(void* ctx, uint8_t request_type, uint8_t request, uint16_t value,
                               uint16_t index, void* data, size_t length, zx_time_t timeout,
                               size_t* out_length);
void usb_device_request_queue(void* ctx, usb_request_t* usb_request);
zx_status_t usb_device_set_interface(void* ctx, uint8_t interface_id, uint8_t alt_setting);
zx_status_t usb_device_set_configuration(void* ctx, uint8_t configuration);

