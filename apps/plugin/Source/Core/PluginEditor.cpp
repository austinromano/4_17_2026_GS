#include "PluginEditor.h"
#include "GhostLog.h"

//==============================================================================
// DragStrip — native JUCE strip for dragging tracks to DAW
//==============================================================================

void DragStrip::setTracks(const juce::Array<TrackItem>& items)
{
    tracks = items;
    repaint();
}

void DragStrip::clear()
{
    tracks.clear();
    repaint();
}

int DragStrip::getTrackAt(int x) const
{
    if (tracks.isEmpty()) return -1;
    int itemW = getWidth() / tracks.size();
    if (itemW < 1) itemW = 1;
    int idx = x / itemW;
    return juce::jlimit(0, tracks.size() - 1, idx);
}

void DragStrip::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF0A0412));

    if (tracks.isEmpty())
    {
        g.setColour(juce::Colour(0xFF6D6F78));
        g.setFont(juce::Font(11.0f));
        g.drawText("Click export to load tracks here for drag-to-DAW",
                   getLocalBounds(), juce::Justification::centred);
        return;
    }

    int itemW = getWidth() / tracks.size();
    for (int i = 0; i < tracks.size(); ++i)
    {
        auto bounds = juce::Rectangle<int>(i * itemW, 0, itemW, getHeight()).reduced(2);

        // Background
        g.setColour(juce::Colour(0xFF1a1a24));
        g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
        g.setColour(juce::Colour(0xFF7C3AED).withAlpha(0.3f));
        g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 1.0f);

        // Track name
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText(tracks[i].name, bounds.reduced(4, 0), juce::Justification::centredLeft);

        // Drag icon
        auto iconArea = bounds.removeFromRight(20);
        g.setColour(juce::Colour(0xFF00FFC8).withAlpha(0.6f));
        g.setFont(juce::Font(9.0f));
        g.drawText(juce::CharPointer_UTF8("\xe2\x87\xa7"), iconArea, juce::Justification::centred); // ⇧
    }
}

void DragStrip::mouseDown(const juce::MouseEvent& e)
{
    dragIndex = getTrackAt(e.x);
}

void DragStrip::mouseDrag(const juce::MouseEvent& e)
{
    if (dragIndex >= 0 && dragIndex < tracks.size()
        && e.getDistanceFromDragStart() > 4
        && tracks[dragIndex].file.existsAsFile())
    {
        GhostLog::write("[DragStrip] Dragging: " + tracks[dragIndex].file.getFullPathName());
        int idx = dragIndex;
        dragIndex = -1;
        juce::DragAndDropContainer::performExternalDragDropOfFiles(
            { tracks[idx].file.getFullPathName() }, false, this);
    }
}

//==============================================================================
// GhostSessionEditor
//==============================================================================

GhostSessionEditor::GhostSessionEditor(GhostSessionProcessor& p)
    : AudioProcessorEditor(&p), proc(p)
{
    auto options = juce::WebBrowserComponent::Options()
        .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
        .withKeepPageLoadedWhenBrowserIsHidden()
        .withNativeIntegrationEnabled()
        .withUserAgent("GhostSession/2.0 JUCE-Plugin")
        .withNativeFunction("exportForDrag", [this](const juce::Array<juce::var>& args,
                                                     juce::WebBrowserComponent::NativeFunctionCompletion complete)
        {
            // Args: array of {url, fileName} objects
            GhostLog::write("[Export] exportForDrag called with " + juce::String(args.size()) + " args");

            juce::Array<DragStrip::TrackItem> items;
            auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                               .getChildFile("GhostSession");
            if (!tempDir.exists()) tempDir.createDirectory();

            for (auto& arg : args)
            {
                auto* obj = arg.getDynamicObject();
                if (!obj) continue;
                auto url = obj->getProperty("url").toString();
                auto name = obj->getProperty("name").toString();
                if (url.isEmpty() || name.isEmpty()) continue;

                auto destFile = tempDir.getChildFile(name);
                if (!destFile.existsAsFile() || destFile.getSize() == 0)
                {
                    GhostLog::write("[Export] Downloading: " + name);
                    juce::URL downloadUrl(url);
                    auto stream = downloadUrl.createInputStream(
                        juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                            .withConnectionTimeoutMs(15000));
                    if (stream)
                    {
                        juce::FileOutputStream fos(destFile);
                        if (fos.openedOk()) { fos.writeFromInputStream(*stream, -1); fos.flush(); }
                    }
                }

                if (destFile.existsAsFile() && destFile.getSize() > 0)
                {
                    GhostLog::write("[Export] Ready: " + name + " (" + juce::String(destFile.getSize()) + " bytes)");
                    items.add({ name, destFile });
                }
            }

            dragStrip.setTracks(items);
            resized(); // Show the strip
            complete(juce::var(items.size()));
        })
        .withUserScript(
            "window.__ghostExportForDrag = function(tracks) {"
            "  if (window.__JUCE__ && window.__JUCE__.backend) {"
            "    try {"
            "      var fn = window.__JUCE__.backend.getNativeFunction('exportForDrag');"
            "      if (fn) { fn.apply(null, tracks); return true; }"
            "    } catch(e) { console.log('Export error:', e); }"
            "  }"
            "  return false;"
            "};"
        );

    webView = std::make_unique<GhostWebView>(options, p);
    addAndMakeVisible(*webView);
    addAndMakeVisible(dragStrip);

    webView->goToURL(getAppUrl());

    setResizable(true, true);
    setResizeLimits(900, 500, 1920, 1200);
    setSize(1100, 720);
}

GhostSessionEditor::~GhostSessionEditor()
{
    if (webView)
    {
        webView->shutdown();
        removeChildComponent(webView.get());
        webView->setVisible(false);
    }
    webView = nullptr;
}

void GhostSessionEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF1A1A2E));

    if (!webView || !webView->isVisible())
    {
        g.setColour(juce::Colour(0xFF8B5CF6));
        g.setFont(juce::Font(18.0f));
        g.drawText("Welcome to Ghost Session",
                   getLocalBounds(), juce::Justification::centred);
    }
}

void GhostSessionEditor::resized()
{
    auto area = getLocalBounds();

    if (dragStrip.hasItems())
    {
        dragStrip.setBounds(area.removeFromBottom(36));
        dragStrip.setVisible(true);
    }
    else
    {
        dragStrip.setVisible(false);
    }

    if (webView)
        webView->setBounds(area);
}

juce::String GhostSessionEditor::getAppUrl() const
{
    juce::String url = "http://localhost:3000";
    auto token = proc.getClient().getAuthToken();
    if (token.isNotEmpty())
        url += "?token=" + juce::URL::addEscapeChars(token, true) + "&mode=plugin";
    else
        url += "?mode=plugin";
    return url;
}
