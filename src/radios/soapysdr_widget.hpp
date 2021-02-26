/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSlider>
#include <QVBoxLayout>
#include <QWidget>

#include <vector>

class SoapySdrRadio;

class SoapySdrWidget : public QWidget
{
  public:
    SoapySdrWidget() = delete;
    SoapySdrWidget(SoapySdrRadio *radio);
    ~SoapySdrWidget() override = default;

    void devicesDiscovered();
    void deviceRead();
    void deviceDestroyed();
    void deviceStarted();
    void deviceStopped();
    void syncUi();

  private:
    SoapySdrRadio *radio_;

    QFormLayout *layout_;

    QComboBox *deviceCombo_;
    QLineEdit *deviceLineEdit_;
    int deviceCustomIdx_;

    QComboBox *channelCombo_;
    QComboBox *antennaCombo_;

    QComboBox *sampleRateCombo_;
    QDoubleSpinBox *sampleRateBox_;
    int sampleRateCustomIdx_;

    QComboBox *bandwidthCombo_;
    QDoubleSpinBox *bandwidthBox_;
    int bandwidthCustomIdx_;

    QCheckBox *agcBox_;
    QSlider *unifiedGainSlider_;
    std::vector<QSlider *> separateGainSliders_;
};
