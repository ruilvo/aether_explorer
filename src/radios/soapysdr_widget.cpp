/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#include "soapysdr_widget.hpp"

#include "qcombobox.h"
#include "qformlayout.h"
#include "qnamespace.h"
#include "qspinbox.h"
#include "soapysdr_radio.hpp"

#include <QAbstractButton>
#include <QDebug>
#include <QString>

SoapySdrWidget::SoapySdrWidget(SoapySdrRadio *radio)
    : radio_(radio), layout_(new QFormLayout(this)), deviceCombo_(new QComboBox(this)),
      deviceLineEdit_(new QLineEdit(this)), deviceCustomIdx_(0),
      channelCombo_(new QComboBox(this)), antennaCombo_(new QComboBox(this)),
      sampleRateCombo_(new QComboBox(this)), sampleRateBox_(new QDoubleSpinBox(this)),
      sampleRateCustomIdx_(0), bandwidthCombo_(new QComboBox(this)),
      bandwidthBox_(new QDoubleSpinBox(this)), bandwidthCustomIdx_(0),
      agcBox_(new QCheckBox(this)), unifiedGainSlider_(new QSlider(Qt::Horizontal, this))
{
    // Setup the layout
    layout_->addRow("Device", deviceCombo_);
    layout_->addRow("Dev. (custom)", deviceLineEdit_);
    layout_->addRow("Channel", channelCombo_);
    layout_->addRow("Antenna", antennaCombo_);
    layout_->addRow("Sample rate", sampleRateCombo_);
    layout_->addRow("S.R. (custom)", sampleRateBox_);
    layout_->addRow("Bandwidth", bandwidthCombo_);
    layout_->addRow("Bw. (custom)", bandwidthBox_);
    layout_->addRow("AGC", agcBox_);
    layout_->addRow("Gain (global)", unifiedGainSlider_);
}
