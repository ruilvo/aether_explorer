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

class SoapySdrSource;

class SoapySdrWidget : public QWidget
{
  public:
    SoapySdrWidget() = delete;
    SoapySdrWidget(SoapySdrSource *source);
    ~SoapySdrWidget() override = default;

    void deviceOpened();
    void deviceStarted();
    void deviceStopped();

  private:
    void configureControls();
    void reconfigureBandwidthCombo();

    void configureChannel();
    void configureAntenna();
    void configureSampleRate();
    void configureBandwidth();
    void configureAgc();
    void configureUnifiedGain();
    void updateUnifiedGain();
    void configureSeparateGains();
    void updateSeparateGains();
    void updateAgc();
    QString convertToUnits(double l_nvalue, QString suffix);

    bool running_;
    int deviceCustomIdx_;
    int deviceRefreshIdx_;
    int sampleRateCustomIdx_;
    int bandwidthCustomIdx_;

    QFormLayout *layout_;

    SoapySdrSource *source_;
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
    std::vector<QSlider *> separateGainSliders_;

  public slots:
    void handleCombo(int value);
    void handleCheck(bool checked);
    void handleSlider(int value);
    void handleSpinBox(double value);
    void handleLineEdit(const QString &value);
};
