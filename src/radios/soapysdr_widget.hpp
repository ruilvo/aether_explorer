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

#include <map>

class SoapySdrRadio;

class SoapySdrWidget : public QWidget
{
  public:
    SoapySdrWidget() = delete;
    SoapySdrWidget(SoapySdrRadio *radio);
    ~SoapySdrWidget() override = default;

    void syncUi();
    void devicesDiscovered();
    void deviceDestroyed();
    void deviceRead();
    void deviceStarted();
    void deviceStopped();

  private:
    SoapySdrRadio *radio_;
    bool running_;

    QFormLayout *layout_;

    QComboBox *deviceCombo_;
    QLineEdit *deviceLineEdit_;

    QComboBox *channelCombo_;
    QComboBox *antennaCombo_;

    QComboBox *sampleRateCombo_;
    QDoubleSpinBox *sampleRateBox_;

    QComboBox *bandwidthCombo_;
    QDoubleSpinBox *bandwidthBox_;

    QCheckBox *agcBox_;
    QSlider *unifiedGainSlider_;
    std::map<QString, QSlider *> separateGainSliders_;
};
