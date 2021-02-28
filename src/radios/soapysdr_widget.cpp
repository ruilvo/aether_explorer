/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#include "soapysdr_widget.hpp"

#include "soapysdr_radio.hpp"

#include <QAbstractButton>
#include <QDebug>
#include <QStandardItemModel>
#include <QString>
#include <QTimer>

#include <algorithm>
#include <iterator>

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects,cert-err58-cpp)
static const QString customTxt{"[Custom]"};

static QString convertToUnits(double l_nvalue, const QString &suffix)
{
    QString unit;
    double value = 0.0;

    if (l_nvalue < 0)
    {
        value = l_nvalue * -1;
    }
    else
    {
        value = l_nvalue;
    }

    // NOLINTNEXTLINE(readability-magic-numbers)
    if (value >= 1000000000 && value < 1000000000000)
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        value = value / 1000000;
        unit = " G" + suffix;
    }
    // NOLINTNEXTLINE(readability-magic-numbers)
    else if (value >= 1000000 && value < 1000000000)
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        value = value / 1000000;
        unit = " M" + suffix;
    }
    // NOLINTNEXTLINE(readability-magic-numbers)
    else if (value >= 1000 && value < 1000000)
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        value = value / 1000;
        unit = " k" + suffix;
    }
    // NOLINTNEXTLINE(readability-magic-numbers)
    else if (value >= 1 && value < 1000)
    {
        value = value * 1;
    }
    // NOLINTNEXTLINE(readability-magic-numbers)
    else if ((value * 1000) >= 1 && value < 1000)
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        value = value * 1000;
        unit = " m" + suffix;
    }
    // NOLINTNEXTLINE(readability-magic-numbers)
    else if ((value * 1000000) >= 1 && value < 1000000)
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        value = value * 1000000;
        // NOLINTNEXTLINE(readability-magic-numbers)
        unit = QString(" ") + QChar(0x00B5) + suffix; // micro
    }
    // NOLINTNEXTLINE(readability-magic-numbers)
    else if ((value * 1000000000) >= 1 && value < 1000000000)
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        value = value * 1000000000;
        unit = " n" + suffix;
    }
    else
    {
        unit = QString(" ") + suffix;
    }

    if (l_nvalue > 0)
    {
        return (QString::number(value) + unit);
    }
    if (l_nvalue < 0)
    {
        return (QString::number(value * -1) + unit);
    }

    return QString::number(0);
}

SoapySdrWidget::SoapySdrWidget(SoapySdrRadio *radio)
    : radio_(radio), running_(false), layout_(new QFormLayout(this)),
      deviceCombo_(new QComboBox(this)), deviceLineEdit_(new QLineEdit(this)),
      channelCombo_(new QComboBox(this)), antennaCombo_(new QComboBox(this)),
      sampleRateCombo_(new QComboBox(this)), sampleRateBox_(new QDoubleSpinBox(this)),
      bandwidthCombo_(new QComboBox(this)), bandwidthBox_(new QDoubleSpinBox(this)),
      agcBox_(new QCheckBox(this)), unifiedGainSlider_(new QSlider(Qt::Horizontal, this))
{
    setMinimumWidth(1); // Makes parent take control of the size;

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

    // Customize widgets
    sampleRateBox_->setMinimum(0);
    sampleRateBox_->setMaximum(500'000'000.00); // NOLINT(readability-magic-numbers)
    sampleRateBox_->setDecimals(2);
    sampleRateBox_->setSuffix(" Sa/s");
    sampleRateBox_->setGroupSeparatorShown(true);
    sampleRateBox_->setKeyboardTracking(false);

    bandwidthBox_->setMinimum(0);
    bandwidthBox_->setMaximum(500'000'000.00); // NOLINT(readability-magic-numbers)
    bandwidthBox_->setDecimals(2);
    bandwidthBox_->setSuffix(" Hz");
    bandwidthBox_->setGroupSeparatorShown(true);
    bandwidthBox_->setKeyboardTracking(false);

    // Disable most of the stuff while there isn't a device chosen
    channelCombo_->setEnabled(false);
    antennaCombo_->setEnabled(false);
    sampleRateCombo_->setEnabled(false);
    sampleRateBox_->setEnabled(false);
    bandwidthCombo_->setEnabled(false);
    bandwidthBox_->setEnabled(false);
    agcBox_->setEnabled(false);
    unifiedGainSlider_->setEnabled(false);

    // Connect (the easy) slots
    connect(channelCombo_, &QComboBox::currentIndexChanged,
            [&](int idx) { radio_->setChannel(static_cast<size_t>(idx)); });
    connect(antennaCombo_, &QComboBox::currentTextChanged,
            [&](const QString &ant) { radio_->setAntenna(ant); });
    connect(agcBox_, &QCheckBox::toggled, [&](bool checked) { radio_->setAgc(checked); });
    connect(unifiedGainSlider_, &QSlider::valueChanged,
            [&](int value) { radio_->setGlobalGain(static_cast<double>(value)); });

    // Handle changing device
    connect(deviceCombo_, &QComboBox::currentTextChanged, [&](const QString &text) {
        if (text != customTxt)
        {
            deviceLineEdit_->setText(radio_->getDeviceStrings()[text]);
            radio_->makeDevice(deviceLineEdit_->text());
            deviceLineEdit_->setEnabled(false);
            return;
        }
        deviceLineEdit_->setEnabled(true);
    });

    // Handle changing device on the line edit;
    connect(deviceLineEdit_, &QLineEdit::returnPressed,
            [&]() { radio_->makeDevice(deviceLineEdit_->text()); });

    // Handle changing sample rate
    connect(sampleRateCombo_, &QComboBox::currentIndexChanged, [&](int idx) {
        if (sampleRateCombo_->currentText() != customTxt)
        {
            auto sr = radio_->getSupportedSampleRatesDiscrete()[static_cast<size_t>(idx)];
            radio_->setSampleRate(sr);
            sampleRateBox_->setEnabled(false);
            sampleRateBox_->blockSignals(true);
            sampleRateBox_->setValue(sr);
            return;
        }
        sampleRateBox_->setEnabled(!running_);
        sampleRateBox_->blockSignals(false);
    });

    connect(sampleRateBox_, &QDoubleSpinBox::valueChanged, [&](double value) {
        sampleRateBox_->blockSignals(true);
        radio_->setSampleRate(value);
        sampleRateBox_->setValue(radio_->getSampleRate());
        sampleRateBox_->blockSignals(false);
    });

    // Handle changing bandwidth
    connect(bandwidthCombo_, &QComboBox::currentIndexChanged, [&](int idx) {
        if (bandwidthCombo_->currentText() != customTxt)
        {
            auto bw = radio_->getSupportedBandwidthsDiscrete()[static_cast<size_t>(idx)];
            radio_->setBandwidth(bw);
            bandwidthBox_->setEnabled(false);
            bandwidthBox_->blockSignals(true);
            bandwidthBox_->setValue(bw);
            return;
        }
        bandwidthBox_->setEnabled(true);
        bandwidthBox_->blockSignals(false);
    });

    connect(bandwidthBox_, &QDoubleSpinBox::valueChanged, [&](double value) {
        bandwidthBox_->blockSignals(true);
        radio_->setBandwidth(value);
        bandwidthBox_->setValue(radio_->getBandwidth());
        bandwidthBox_->blockSignals(false);
    });
}

void SoapySdrWidget::syncUi()
{
    channelCombo_->blockSignals(true);
    channelCombo_->setCurrentIndex(static_cast<int>(radio_->getChannel()));
    channelCombo_->blockSignals(false);

    antennaCombo_->blockSignals(true);
    antennaCombo_->setCurrentText(radio_->getAntenna());
    antennaCombo_->blockSignals(false);

    sampleRateCombo_->blockSignals(true);
    sampleRateBox_->blockSignals(true);
    auto srs = radio_->getSupportedSampleRatesDiscrete();
    auto csr = radio_->getSampleRate();
    sampleRateBox_->setValue(csr);
    auto sridx = std::find(srs.begin(), srs.end(), csr);
    if (sridx != srs.end())
    {
        sampleRateCombo_->setCurrentIndex(
            static_cast<int>(std::distance(srs.begin(), sridx)));
        sampleRateBox_->setEnabled(false);
    }
    else if (sampleRateCombo_->count() > 0)
    {
        sampleRateCombo_->setCurrentIndex(sampleRateCombo_->count() - 1);
        sampleRateBox_->setEnabled(!running_);
        sampleRateBox_->blockSignals(false);
    }
    sampleRateCombo_->blockSignals(false);

    bandwidthCombo_->blockSignals(true);
    bandwidthBox_->blockSignals(true);
    auto sbw = radio_->getSupportedBandwidthsDiscrete();
    auto cbw = radio_->getBandwidth();
    bandwidthBox_->setValue(cbw);
    auto cbidx = std::find(sbw.begin(), sbw.end(), cbw);
    auto *model = dynamic_cast<QStandardItemModel *>(bandwidthCombo_->model());
    for (auto i = 0U; i < sbw.size(); i++)
    {
        model->item(static_cast<int>(i), 0)->setEnabled(sbw[i] <= csr);
    }
    if (cbidx != sbw.end())
    {
        bandwidthCombo_->setCurrentIndex(
            static_cast<int>(std::distance(sbw.begin(), cbidx)));
        bandwidthBox_->setEnabled(false);
    }
    else if (bandwidthCombo_->count() > 0)
    {
        bandwidthCombo_->setCurrentIndex(bandwidthCombo_->count() - 1);
        bandwidthBox_->setEnabled(bandwidthCombo_->count() > 1);
        bandwidthBox_->blockSignals(false);
    }
    bandwidthCombo_->blockSignals(false);

    agcBox_->blockSignals(true);
    agcBox_->setEnabled(radio_->hasAgc());
    agcBox_->setChecked(radio_->getAgc());
    agcBox_->blockSignals(false);

    unifiedGainSlider_->blockSignals(true);
    auto ugainrange = radio_->getGlobalGainRange();
    unifiedGainSlider_->setMinimum(static_cast<int>(ugainrange.minimum));
    unifiedGainSlider_->setMaximum(static_cast<int>(ugainrange.maximum));
    unifiedGainSlider_->setValue(static_cast<int>(radio_->getGlobalGain()));
    unifiedGainSlider_->blockSignals(false);
    unifiedGainSlider_->setEnabled(unifiedGainSlider_->minimum() !=
                                   unifiedGainSlider_->maximum());

    auto specgains = radio_->getSpecificGains();
    for (const auto &gain : radio_->getSpecificGainRanges())
    {
        separateGainSliders_[gain.first]->blockSignals(true);
        separateGainSliders_[gain.first]->setMinimum(
            static_cast<int>(gain.second.minimum));
        separateGainSliders_[gain.first]->setMaximum(
            static_cast<int>(gain.second.maximum));
        separateGainSliders_[gain.first]->setValue(
            static_cast<int>(specgains[gain.first]));
        separateGainSliders_[gain.first]->blockSignals(false);
        separateGainSliders_[gain.first]->setEnabled(
            separateGainSliders_[gain.first]->minimum() !=
            separateGainSliders_[gain.first]->maximum());
    }
}

void SoapySdrWidget::devicesDiscovered()
{
    // Disable most of the stuff while there isn't a device chosen
    channelCombo_->setEnabled(false);
    antennaCombo_->setEnabled(false);
    sampleRateCombo_->setEnabled(false);
    sampleRateBox_->setEnabled(false);
    bandwidthCombo_->setEnabled(false);
    bandwidthBox_->setEnabled(false);
    agcBox_->setEnabled(false);
    unifiedGainSlider_->setEnabled(false);
    for (const auto &slider : separateGainSliders_)
    {
        separateGainSliders_[slider.first]->setEnabled(false);
    }

    deviceCombo_->blockSignals(true);
    deviceCombo_->clear();
    auto devices = radio_->getDeviceStrings();
    for (const auto &dev : devices)
    {
        deviceCombo_->addItem(dev.first);
    }
    deviceCombo_->addItem(customTxt);
    deviceCombo_->blockSignals(false);

    deviceLineEdit_->clear();

    if (deviceCombo_->count() > 0)
    {
        deviceCombo_->currentTextChanged(deviceCombo_->itemText(0));
    }
}

void SoapySdrWidget::deviceDestroyed()
{
    // Disable most of the stuff while there isn't a device chosen
    channelCombo_->setEnabled(false);
    antennaCombo_->setEnabled(false);
    sampleRateCombo_->setEnabled(false);
    sampleRateBox_->setEnabled(false);
    bandwidthCombo_->setEnabled(false);
    bandwidthBox_->setEnabled(false);
    agcBox_->setEnabled(false);
    unifiedGainSlider_->setEnabled(false);
    for (const auto &slider : separateGainSliders_)
    {
        layout_->removeRow(separateGainSliders_[slider.first]);
    }
    separateGainSliders_.clear();

    if (parentWidget() != nullptr)
    {
        // This is a "funny" hack that "refits" the parent widget to changing widget
        // sizes. You can't call this directly because it must happen in the event loop.
        // The goal is to only resize in the vertical direction.
        QTimer::singleShot(
            /* Event loop delay */ 10, // NOLINT(readability-magic-numbers)
            [&]() {
                auto sHint = parentWidget()->sizeHint();
                auto cSize = parentWidget()->size();
                parentWidget()->resize(cSize.width(), sHint.height());
            });
    }
}

void SoapySdrWidget::deviceRead()
{
    deviceDestroyed();

    channelCombo_->blockSignals(true);
    channelCombo_->clear();
    auto nchans = radio_->getChannelCount();
    for (auto i = 0U; i < nchans; i++)
    {
        channelCombo_->addItem(QString("Channel %1").arg(i));
    }
    if (channelCombo_->count() > 1)
    {
        channelCombo_->blockSignals(false);
        channelCombo_->setEnabled(true);
    }
    else
    {
        channelCombo_->setEnabled(false);
    }

    antennaCombo_->blockSignals(true);
    antennaCombo_->clear();
    auto ants = radio_->getSupportedAntennas();
    for (const auto &ant : ants)
    {
        antennaCombo_->addItem(ant);
    }
    if (antennaCombo_->count() > 1)
    {
        antennaCombo_->blockSignals(false);
        antennaCombo_->setEnabled(true);
    }
    else
    {
        antennaCombo_->setEnabled(false);
    }

    sampleRateCombo_->blockSignals(true);
    sampleRateBox_->blockSignals(true);
    sampleRateCombo_->clear();
    auto sampleRates = radio_->getSupportedSampleRatesDiscrete();
    for (auto sampleRate : sampleRates)
    {
        sampleRateCombo_->addItem(convertToUnits(sampleRate, QString("Sa/s")));
    }
    if (!radio_->getSupportedSampleRatesRanges().empty())
    {
        sampleRateCombo_->addItem(customTxt);
    }
    sampleRateCombo_->blockSignals(false);
    sampleRateBox_->blockSignals(false);
    sampleRateCombo_->setEnabled(true);

    bandwidthCombo_->blockSignals(true);
    bandwidthCombo_->clear();
    auto bandwidths = radio_->getSupportedBandwidthsDiscrete();
    for (auto bandwidth : bandwidths)
    {
        bandwidthCombo_->addItem(convertToUnits(bandwidth, QString("Hz")));
    }
    if (!radio_->getSupportedBandwidthsRanges().empty())
    {
        bandwidthCombo_->addItem(customTxt);
    }
    bandwidthCombo_->blockSignals(false);
    bandwidthCombo_->setEnabled(bandwidthCombo_->count() > 1);

    unifiedGainSlider_->setEnabled(true);

    // Create sliders
    for (const auto &gain : radio_->getSpecificGainRanges())
    {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        separateGainSliders_[gain.first] = new QSlider(Qt::Horizontal);
        layout_->addRow(gain.first, separateGainSliders_[gain.first]);
    }

    syncUi();
}

void SoapySdrWidget::deviceStarted()
{
    channelCombo_->setEnabled(false);
    sampleRateCombo_->setEnabled(false);
    sampleRateBox_->setEnabled(false);
    deviceCombo_->setEnabled(false);
    deviceLineEdit_->setEnabled(false);
    running_ = true;
}

void SoapySdrWidget::deviceStopped()
{
    channelCombo_->setEnabled(true);
    sampleRateCombo_->setEnabled(true);
    sampleRateBox_->setEnabled(true);
    deviceCombo_->setEnabled(true);
    deviceLineEdit_->setEnabled(true);
    running_ = false;

    syncUi();
}
