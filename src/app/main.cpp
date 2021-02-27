/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#include "ISource.hpp"
#include "ISourceListener.hpp"
#include "soapysdr_radio.hpp"
#include "source_factory.hpp"
#include "source_listeners_collection.hpp"
#include "source_manager.hpp"

#include <QApplication>
#include <QDebug>

#include <memory>

class BasicSourceListener : public ISourceListener,
                            public std::enable_shared_from_this<BasicSourceListener>
{
  public:
    BasicSourceListener()
    {
        qDebug() << "Initialized source listener";
    };
    ~BasicSourceListener() override
    {
        qDebug() << "Destroyed source listener";
    };
    void setSampleRate(double sampleRate) override
    {
        qDebug() << "Sample rate changed to " << sampleRate;
    };
    void setCentreFrequency(double centreFrequency) override
    {
        qDebug() << "Centre frequency changed to " << centreFrequency;
    };
    void receiveSamples(std::vector<std::complex<float>> &samples) override
    {
        qDebug() << "Received " << samples.size() << " samples";
    };

    std::shared_ptr<BasicSourceListener> getSharedPtr()
    {
        return shared_from_this();
    };
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto listenersCollection = SourceListenersCollection();
    /*
     * Now initialize the listeners...
     */
    auto basicListener = std::make_shared<BasicSourceListener>();
    listenersCollection.subscribe(basicListener->getSharedPtr());

    auto sourceFactory = SourceFactory();
    /*
     * Now register sources...
     */
    sourceFactory.registerSource("SoapySDR",
                                []() { return std::make_unique<SoapySdrRadio>(); });

    auto sourceManager =
        SourceManager(std::move(sourceFactory), std::move(listenersCollection));

    // In the future this would be part of a larger main Window, of course...
    sourceManager.getWidget()->show();

    return QApplication::exec();
}
