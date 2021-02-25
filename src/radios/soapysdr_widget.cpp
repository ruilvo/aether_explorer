/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#include "soapysdr_widget.hpp"

#include "soapysdr_source.hpp"

#include <QAbstractButton>
#include <QDebug>
#include <QStandardItemModel>
#include <QString>
#include <Qt>

SoapySdrWidget::SoapySdrWidget(SoapySdrSource *source)
    : running_{false}, deviceCustomIdx_{-1}, sampleRateCustomIdx_{-1},
      bandwidthCustomIdx_{-1}, source_{nullptr}, deviceCombo_{nullptr}
{
    source_ = source;

    layout_ = new QFormLayout(this);

    deviceCombo_ = new QComboBox();
    layout_->addRow("Device", deviceCombo_);
    connect(deviceCombo_, &QComboBox::currentIndexChanged, this,
            &SoapySdrWidget::handleCombo);

    deviceLineEdit_ = new QLineEdit();
    layout_->addRow("Dev. str.", deviceLineEdit_);
    connect(deviceLineEdit_, &QLineEdit::textChanged, this,
            &SoapySdrWidget::handleLineEdit);

    layout_->addRow("  ", (QWidget *)nullptr);

    channelCombo_ = new QComboBox();
    layout_->addRow("Channel", channelCombo_);
    connect(channelCombo_, &QComboBox::currentIndexChanged, this,
            &SoapySdrWidget::handleCombo);

    antennaCombo_ = new QComboBox();
    layout_->addRow("Antenna", antennaCombo_);
    connect(antennaCombo_, &QComboBox::currentIndexChanged, this,
            &SoapySdrWidget::handleCombo);

    sampleRateCombo_ = new QComboBox();
    layout_->addRow("Sample Rate", sampleRateCombo_);
    connect(sampleRateCombo_, &QComboBox::currentIndexChanged, this,
            &SoapySdrWidget::handleCombo);

    sampleRateBox_ = new QDoubleSpinBox();
    sampleRateBox_->setMinimum(0);
    sampleRateBox_->setMaximum(100000000.0);
    sampleRateBox_->setDecimals(2);
    sampleRateBox_->setSuffix(" Sa/s");
    sampleRateBox_->setGroupSeparatorShown(true);
    sampleRateBox_->setKeyboardTracking(false);
    layout_->addRow("S.R. (custom)", sampleRateBox_);
    connect(sampleRateBox_, &QDoubleSpinBox::valueChanged, this,
            &SoapySdrWidget::handleSpinBox);

    bandwidthCombo_ = new QComboBox();
    layout_->addRow("Bandwidth", bandwidthCombo_);
    connect(bandwidthCombo_, &QComboBox::currentIndexChanged, this,
            &SoapySdrWidget::handleCombo);

    bandwidthBox_ = new QDoubleSpinBox();
    bandwidthBox_->setMinimum(0);
    bandwidthBox_->setMaximum(100000000.0);
    bandwidthBox_->setDecimals(2);
    bandwidthBox_->setSuffix(" Hz");
    bandwidthBox_->setGroupSeparatorShown(true);
    bandwidthBox_->setKeyboardTracking(false);
    layout_->addRow("Bw. (custom)", bandwidthBox_);
    connect(bandwidthBox_, &QDoubleSpinBox::valueChanged, this,
            &SoapySdrWidget::handleSpinBox);

    agcBox_ = new QCheckBox();
    layout_->addRow("AGC", agcBox_);
    connect(agcBox_, &QCheckBox::toggled, this, &SoapySdrWidget::handleCheck);
    unifiedGainSlider_ = new QSlider(Qt::Horizontal);
    layout_->addRow("Gain (unified)", unifiedGainSlider_);
    connect(unifiedGainSlider_, &QSlider::valueChanged, this,
            &SoapySdrWidget::handleSlider);
    separateGainSliders_.clear();
}

void SoapySdrWidget::deviceOpened()
{

    deviceCombo_->blockSignals(true);
    deviceCombo_->clear();

    auto devNames = source_->getDeviceNames();
    for (auto devName : devNames)
    {
        deviceCombo_->addItem(devName);
    }
    deviceCombo_->addItem(QString{"[Custom]"});
    deviceCustomIdx_ = deviceCombo_->findText("[Custom]");
    deviceCombo_->addItem(QString{"[Refresh]"});
    deviceRefreshIdx_ = deviceCombo_->findText(QString{"[Refresh]"});
    deviceCombo_->blockSignals(false);
    deviceCombo_->setCurrentIndex(0);
    deviceCombo_->currentIndexChanged(0);
}

void SoapySdrWidget::configureChannel()
{
    channelCombo_->blockSignals(true);
    channelCombo_->clear();
    auto nChans = source_->getChannelCount();
    for (auto i = 0u; i < nChans; i++)
    {
        channelCombo_->addItem(QString("Channel %1").arg(i));
    }
    if (channelCombo_->count() > 0)
    {
        channelCombo_->setCurrentIndex(source_->getChannel());
    }
    if (channelCombo_->count() > 1)
    {
        channelCombo_->blockSignals(false);
    }
}

void SoapySdrWidget::configureAntenna()
{
    antennaCombo_->blockSignals(true);
    antennaCombo_->clear();
    auto antennas = source_->getSupportedAntennas();
    for (auto antenna : antennas)
    {
        antennaCombo_->addItem(antenna);
    }
    if (antennaCombo_->count() > 0)
    {
        antennaCombo_->setCurrentText(source_->getAntenna());
    }
    if (antennaCombo_->count() > 1)
    {
        antennaCombo_->blockSignals(false);
    }
}

void SoapySdrWidget::configureSampleRate()
{
    sampleRateCombo_->blockSignals(true);
    sampleRateBox_->blockSignals(true);
    sampleRateCombo_->clear();
    auto sampleRates = source_->getSupportedSampleRatesDiscrete();
    for (auto sampleRate : sampleRates)
    {
        sampleRateCombo_->addItem(convertToUnits(sampleRate, QString("Sa/s")));
    }
    if (sampleRates.size() > 0)
    {
        sampleRateCombo_->setCurrentText(
            convertToUnits(source_->getSampleRate(), QString("Sa/s")));
        sampleRateBox_->setValue(source_->getSampleRate());
    }
    if (source_->getSupportedSampleRatesRanges().size() > 0)
    {
        sampleRateCombo_->addItem("Custom");
        sampleRateCustomIdx_ = sampleRateCombo_->findText("Custom");
        sampleRateBox_->setValue(source_->getSampleRate());
        sampleRateBox_->blockSignals(false);
    }
    if (sampleRateCombo_->count() > 1)
    {
        sampleRateCombo_->blockSignals(false);
    }
}

void SoapySdrWidget::configureBandwidth()
{
    bandwidthCombo_->setEnabled(true);
    bandwidthCombo_->blockSignals(true);
    bandwidthBox_->blockSignals(true);
    bandwidthCombo_->clear();
    bandwidthBox_->clear();
    auto bandwidths = source_->getSupportedBandwidthsDiscrete();
    for (auto bandwidth : bandwidths)
    {
        bandwidthCombo_->addItem(convertToUnits(bandwidth, QString("Hz")));
    }
    if (source_->getSupportedBandwidthsRanges().size() > 0)
    {
        bandwidthCombo_->addItem("Custom");
        bandwidthCustomIdx_ = bandwidthCombo_->findText("Custom");
        bandwidthBox_->setValue(source_->getBandwidth());
        bandwidthBox_->blockSignals(false);
    }
    if (bandwidthCombo_->count() > 1)
    {
        bandwidthCombo_->blockSignals(false);
    }
    else
    {
        bandwidthCombo_->setEnabled(false);
    }
}

void SoapySdrWidget::configureAgc()
{
    agcBox_->blockSignals(true);
    agcBox_->setChecked(source_->getAgc());
    if (source_->hasAgc())
    {
        agcBox_->blockSignals(false);
    }
}

void SoapySdrWidget::updateAgc()
{
    agcBox_->blockSignals(true);
    agcBox_->setChecked(source_->getAgc());
    agcBox_->blockSignals(false);
}

void SoapySdrWidget::updateUnifiedGain()
{
    unifiedGainSlider_->blockSignals(true);
    unifiedGainSlider_->setValue(static_cast<int>(source_->getGlobalGain()));
    unifiedGainSlider_->blockSignals(false);

    updateAgc();
}

void SoapySdrWidget::configureUnifiedGain()
{
    unifiedGainSlider_->blockSignals(true);
    auto globalRange = source_->getGlobalGainRange();
    unifiedGainSlider_->setMinimum(static_cast<int>(globalRange.minimum));
    unifiedGainSlider_->setMaximum(static_cast<int>(globalRange.maximum));
    unifiedGainSlider_->blockSignals(false);

    updateUnifiedGain();
}

void SoapySdrWidget::updateSeparateGains()
{
    auto gainValues = source_->getSpecificGains();

    for (auto i = 0u; i < separateGainSliders_.size(); i++)
    {
        separateGainSliders_[i]->blockSignals(true);
        separateGainSliders_[i]->setValue(static_cast<int>(gainValues[i]));
        separateGainSliders_[i]->blockSignals(false);
    }

    updateAgc();
}

void SoapySdrWidget::configureSeparateGains()
{
    for (auto widget : separateGainSliders_)
    {
        layout_->removeRow(widget);
    }

    separateGainSliders_.clear();

    auto gainNames = source_->getSpecificGainNames();
    auto gainRanges = source_->getSpecificGainRanges();
    if (gainNames.size() < 2)
    {
        return;
    }
    for (auto i = 0u; i < gainNames.size(); i++)
    {
        separateGainSliders_.push_back(new QSlider(Qt::Horizontal));

        bool sliderUseless = gainRanges[i].minimum == gainRanges[i].maximum;

        separateGainSliders_[i]->blockSignals(true);
        separateGainSliders_[i]->setMinimum(static_cast<int>(gainRanges[i].minimum));
        separateGainSliders_[i]->setMaximum(static_cast<int>(gainRanges[i].maximum));

        if (!sliderUseless)
        {
            layout_->addRow(gainNames[i], separateGainSliders_[i]);
            connect(separateGainSliders_[i], &QSlider::valueChanged, this,
                    &SoapySdrWidget::handleSlider);

            separateGainSliders_[i]->blockSignals(false);
        }
    }

    updateSeparateGains();
}

void SoapySdrWidget::deviceStarted()
{
    running_ = true;

    configureChannel();
    configureAntenna();
    configureSampleRate();
    configureBandwidth();
    configureAgc();
    configureUnifiedGain();
    configureSeparateGains();

    reconfigureBandwidthCombo();

    configureControls();
}

void SoapySdrWidget::deviceStopped()
{
    running_ = false;

    configureControls();
}

void SoapySdrWidget::handleCombo(int value)
{
    Q_UNUSED(value)

    auto combo = dynamic_cast<QComboBox *>(sender());

    if (combo == deviceCombo_)
    {
        auto index = deviceCombo_->currentIndex();

        if (index < deviceCustomIdx_)
        {
            auto text = source_->getDeviceStrings()[index];
            deviceLineEdit_->blockSignals(true);
            deviceLineEdit_->setText(text);
            deviceLineEdit_->blockSignals(false);
            source_->setDeviceString(text);
        }
        else if (index == deviceRefreshIdx_)
        {
            source_->open();
        }
    }

    if (combo == channelCombo_)
    {
        auto index = channelCombo_->currentIndex();
        source_->setChannel(index);
    }

    if (combo == antennaCombo_)
    {
        auto index = antennaCombo_->currentIndex();
        source_->setAntenna(source_->getSupportedAntennas()[index]);
    }

    if (combo == sampleRateCombo_)
    {
        auto index = sampleRateCombo_->currentIndex();
        if (index != sampleRateCustomIdx_)
        {
            source_->setSampleRate(source_->getSupportedSampleRatesDiscrete()[index]);
            sampleRateBox_->blockSignals(true);
            sampleRateBox_->setValue(source_->getSampleRate());
            sampleRateBox_->blockSignals(false);
        }

        reconfigureBandwidthCombo();
    }

    if (combo == bandwidthCombo_)
    {
        auto index = bandwidthCombo_->currentIndex();
        if (index != bandwidthCustomIdx_)
        {
            source_->setBandwidth(source_->getSupportedBandwidthsDiscrete()[index]);

            reconfigureBandwidthCombo();
        }
    }
}

void SoapySdrWidget::handleCheck(bool checked)
{
    Q_UNUSED(checked)

    auto box = dynamic_cast<QAbstractButton *>(sender());

    if (box == agcBox_)
    {
        source_->setAgc(agcBox_->isChecked());
    }
}

void SoapySdrWidget::handleSlider(int value)
{
    Q_UNUSED(value)

    auto slider = dynamic_cast<QSlider *>(sender());

    if (slider == unifiedGainSlider_)
    {
        source_->setGlobalGain(static_cast<double>(unifiedGainSlider_->value()));
        updateSeparateGains();
    }

    auto gainNames = source_->getSpecificGainNames();
    for (auto i = 0u; i < separateGainSliders_.size(); i++)
    {
        if (separateGainSliders_[i] == slider)
        {
            source_->setSpecificGain(
                gainNames[i], static_cast<double>(separateGainSliders_[i]->value()));
            updateUnifiedGain();
        }
    }
}

void SoapySdrWidget::handleSpinBox(double value)
{
    Q_UNUSED(value)

    auto spinBox = dynamic_cast<QDoubleSpinBox *>(sender());

    if (spinBox == sampleRateBox_)
    {
        source_->setSampleRate(spinBox->value());

        sampleRateBox_->blockSignals(true);
        sampleRateBox_->setValue(source_->getSampleRate());
        sampleRateBox_->blockSignals(false);
    }
    else if (spinBox == bandwidthBox_)
    {
        source_->setBandwidth(spinBox->value());

        bandwidthBox_->blockSignals(true);
        bandwidthBox_->setValue(source_->getBandwidth());
        bandwidthBox_->blockSignals(false);
    }
}

void SoapySdrWidget::handleLineEdit(const QString &value)
{
    Q_UNUSED(value)

    auto lineEdit = dynamic_cast<QLineEdit *>(sender());

    if (lineEdit == deviceLineEdit_)
    {
        auto text = lineEdit->text();
        deviceCombo_->setCurrentIndex(deviceCustomIdx_);
        source_->setDeviceString(text);
    }
}

void SoapySdrWidget::configureControls()
{
    channelCombo_->setEnabled(running_ && source_->getChannelCount() > 1);
    antennaCombo_->setEnabled(running_ && source_->getSupportedAntennas().size() > 1);
    sampleRateCombo_->setEnabled(running_ && sampleRateCombo_->count() > 1);
    sampleRateBox_->setEnabled(running_ &&
                               sampleRateCombo_->currentIndex() == sampleRateCustomIdx_);
    bandwidthCombo_->setEnabled(running_ && bandwidthCombo_->count() > 1);
    bandwidthBox_->setEnabled(running_ && bandwidthCombo_->count() > 0 &&
                              bandwidthCombo_->currentIndex() == bandwidthCustomIdx_);
    agcBox_->setEnabled(running_ && source_->hasAgc());

    deviceCombo_->setEnabled(!running_);
    deviceLineEdit_->setEnabled(!running_);

    return;
}

void SoapySdrWidget::reconfigureBandwidthCombo()
{
    if (!bandwidthCombo_->isEnabled())
    {
        return;
    }

    bandwidthCombo_->blockSignals(true);
    bandwidthBox_->blockSignals(true);

    auto model = dynamic_cast<QStandardItemModel *>(bandwidthCombo_->model());

    auto supFs = source_->getSupportedSampleRatesDiscrete();
    auto supBw = source_->getSupportedBandwidthsDiscrete();
    auto currBw = source_->getBandwidth();

    auto currFs = supFs[sampleRateCombo_->currentIndex()];

    for (auto i = 0; i < bandwidthCombo_->count(); i++)
    {
        if (i == bandwidthCustomIdx_)
        {
            model->item(i, 0)->setEnabled(true);
            continue;
        }

        bool toEnable = supBw[i] <= currFs;
        model->item(i, 0)->setEnabled(toEnable);

        if (supBw[i] == currBw)
        {
            bandwidthCombo_->setCurrentIndex(i);
        }
    }

    bandwidthBox_->setValue(currBw);

    bandwidthCombo_->blockSignals(false);
    bandwidthBox_->blockSignals(false);
}

QString SoapySdrWidget::convertToUnits(double l_nvalue, QString suffix)
{
    QString unit;
    double value;

    if (l_nvalue < 0)
    {
        value = l_nvalue * -1;
    }
    else
    {
        value = l_nvalue;
    }

    if (value >= 1000000000 && value < 1000000000000)
    {
        value = value / 1000000;
        unit = " G" + suffix;
    }
    else if (value >= 1000000 && value < 1000000000)
    {
        value = value / 1000000;
        unit = " M" + suffix;
    }
    else if (value >= 1000 && value < 1000000)
    {
        value = value / 1000;
        unit = " k" + suffix;
    }
    else if (value >= 1 && value < 1000)
    {
        value = value * 1;
    }
    else if ((value * 1000) >= 1 && value < 1000)
    {
        value = value * 1000;
        unit = " m" + suffix;
    }
    else if ((value * 1000000) >= 1 && value < 1000000)
    {
        value = value * 1000000;
        unit = QString(" ") + QChar(0x00B5) + suffix; // micro
    }
    else if ((value * 1000000000) >= 1 && value < 1000000000)
    {
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
    else if (l_nvalue < 0)
    {
        return (QString::number(value * -1) + unit);
    }

    return QString::number(0);
}
