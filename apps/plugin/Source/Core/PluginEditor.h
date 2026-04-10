#pragma once

#include "JuceHeader.h"
#include "PluginProcessor.h"
#include "../UI/GhostWebView.h"

//==============================================================================
/** A native JUCE strip that shows exported tracks for drag-to-DAW. */
class DragStrip : public juce::Component
{
public:
    struct TrackItem
    {
        juce::String name;
        juce::File file;
    };

    void setTracks(const juce::Array<TrackItem>& items);
    void clear();
    bool hasItems() const { return tracks.size() > 0; }

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

private:
    juce::Array<TrackItem> tracks;
    int dragIndex = -1;
    int getTrackAt(int x) const;
};

//==============================================================================
class GhostSessionEditor : public juce::AudioProcessorEditor,
                           public juce::DragAndDropContainer
{
public:
    explicit GhostSessionEditor(GhostSessionProcessor&);
    ~GhostSessionEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    DragStrip& getDragStrip() { return dragStrip; }

private:
    GhostSessionProcessor& proc;
    std::unique_ptr<GhostWebView> webView;
    DragStrip dragStrip;

    juce::String getAppUrl() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GhostSessionEditor)
};
