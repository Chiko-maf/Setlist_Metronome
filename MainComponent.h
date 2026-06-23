#pragma once
#include <JuceHeader.h>
#include <atomic>
#include <cmath>
#include <vector>

//==============================================================================
// 1. Data Model
//==============================================================================
struct Song
{
    juce::String title { "New Song" };
    double bpm { 120.0 };
    int ts { 4 };
    juce::String notes { "Intro: \nVerse: \nChorus: " };
    juce::Colour color { 0xffff7b00 };

    juce::XmlElement* toXml() const
    {
        auto* xml = new juce::XmlElement("SONG");
        xml->setAttribute("title", title);
        xml->setAttribute("bpm", bpm);
        xml->setAttribute("ts", ts);
        xml->setAttribute("notes", notes);
        xml->setAttribute("color", color.toString());
        return xml;
    }

    static Song fromXml(juce::XmlElement* xml)
    {
        Song s;
        if (xml != nullptr) {
            s.title = xml->getStringAttribute("title", "New Song");
            s.bpm = xml->getDoubleAttribute("bpm", 120.0);
            s.ts = xml->getIntAttribute("ts", 4);
            s.notes = xml->getStringAttribute("notes", "");
            s.color = juce::Colour::fromString(xml->getStringAttribute("color", "ffff7b00"));
        }
        return s;
    }
};

//==============================================================================
// 2. The Click Synth
//==============================================================================
class WorshipClickSynth
{
public:
    void prepare(double sampleRate)
    {
        currentSampleRate = sampleRate;
        phase = 0.0;
        amplitude = 0.0;
    }

    void trigger(bool isDownbeat)
    {
        targetFrequency = isDownbeat ? 1000.0 : 500.0;
        phase = 0.0;
        amplitude = 1.0;
    }

    float getNextSample()
    {
        if (amplitude < 0.0001) return 0.0f;

        amplitude *= 0.996;

        phase += juce::MathConstants<double>::twoPi * targetFrequency / currentSampleRate;
        if (phase > juce::MathConstants<double>::twoPi)
            phase -= juce::MathConstants<double>::twoPi;

        return static_cast<float>(std::sin(phase) * amplitude);
    }

private:
    double currentSampleRate = 44100.0;
    double phase = 0.0;
    double amplitude = 0.0;
    double targetFrequency = 500.0;
};

//==============================================================================
// 3. UI Components
//==============================================================================
class StageTile : public juce::Component
{
public:
    StageTile(const Song& s, int idx, std::function<void(int)> onClickCallback)
        : song(s), index(idx), onClick(std::move(onClickCallback)) {}

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        juce::Colour bgColor = isActive ? juce::Colour(0xff3e3e3e) : juce::Colour(0xff2d2d2d);

        if (isActive && pulseIntensity > 0.0f)
            bgColor = bgColor.interpolatedWith(song.color, pulseIntensity);

        g.setColour(bgColor);
        g.fillRoundedRectangle(bounds, 8.0f);

        if (isActive)
        {
            g.setColour(song.color);
            g.drawRoundedRectangle(bounds, 8.0f, 2.0f);
        }

        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(24.0f, juce::Font::bold));
        g.drawText(song.title, getLocalBounds().reduced(15, 10).removeFromTop(30), juce::Justification::centredLeft);

        g.setColour(juce::Colours::grey);
        g.setFont(17.0f);

        // Using the safe Unicode bullet point (0x2022) to prevent Windows encoding errors
        g.drawText(juce::String(song.bpm) + " BPM   " + juce::String::charToString(0x2022) + "   " + juce::String(song.ts) + "/4",
                   getLocalBounds().reduced(15, 10).removeFromBottom(25), juce::Justification::centredLeft);
    }

    void mouseDown(const juce::MouseEvent&) override { onClick(index); }
    void mouseEnter(const juce::MouseEvent&) override { setMouseCursor(juce::MouseCursor::PointingHandCursor); }

    void setActive(bool active) { isActive = active; repaint(); }
    void setPulse(float intensity) { pulseIntensity = intensity; repaint(); }

private:
    const Song& song;
    int index;
    bool isActive = false;
    float pulseIntensity = 0.0f;
    std::function<void(int)> onClick;
};

class EditorRow : public juce::Component
{
public:
    EditorRow(Song& s, std::function<void()> onDel, std::function<void()> onEdit)
        : song(s), onDelete(std::move(onDel)), onDataChanged(std::move(onEdit))
    {
        setupEditor(titleEd, song.title);
        setupEditor(bpmEd, juce::String(song.bpm));
        setupEditor(tsEd, juce::String(song.ts));

        colorPicker.addItem("Orange", 1);
        colorPicker.addItem("Blue", 2);
        colorPicker.addItem("Green", 3);
        colorPicker.addItem("Pink", 4);
        colorPicker.addItem("Purple", 5);
        colorPicker.addItem("Yellow", 6);

        if (song.color == juce::Colour(0xff00a8ff)) colorPicker.setSelectedId(2, juce::dontSendNotification);
        else if (song.color == juce::Colour(0xff4cd137)) colorPicker.setSelectedId(3, juce::dontSendNotification);
        else if (song.color == juce::Colour(0xffff4757)) colorPicker.setSelectedId(4, juce::dontSendNotification);
        else if (song.color == juce::Colour(0xff9c88ff)) colorPicker.setSelectedId(5, juce::dontSendNotification);
        else if (song.color == juce::Colour(0xfffbc531)) colorPicker.setSelectedId(6, juce::dontSendNotification);
        else colorPicker.setSelectedId(1, juce::dontSendNotification);

        colorPicker.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1e1e1e));
        colorPicker.setColour(juce::ComboBox::textColourId, song.color);
        colorPicker.setJustificationType(juce::Justification::centred);
        colorPicker.onChange = [this]() {
            updateData();
            colorPicker.setColour(juce::ComboBox::textColourId, song.color);
        };
        addAndMakeVisible(colorPicker);

        delBtn.setButtonText("X");
        delBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        delBtn.setColour(juce::TextButton::textColourOnId, juce::Colour(0xffff4a4a));
        delBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffff4a4a));
        delBtn.onClick = onDelete;
        addAndMakeVisible(delBtn);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(2);
        delBtn.setBounds(area.removeFromRight(40).reduced(2));
        colorPicker.setBounds(area.removeFromRight(90).reduced(2));
        tsEd.setBounds(area.removeFromRight(60).reduced(2));
        bpmEd.setBounds(area.removeFromRight(80).reduced(2));
        titleEd.setBounds(area.reduced(2));
    }

private:
    Song& song;
    std::function<void()> onDelete;
    std::function<void()> onDataChanged;

    juce::TextEditor titleEd, bpmEd, tsEd;
    juce::ComboBox colorPicker;
    juce::TextButton delBtn;

    void setupEditor(juce::TextEditor& ed, const juce::String& text)
    {
        addAndMakeVisible(ed);
        ed.setText(text, juce::dontSendNotification);
        ed.setFont(juce::Font(18.0f));
        ed.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff1e1e1e));
        ed.setColour(juce::TextEditor::textColourId, juce::Colours::white);
        ed.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xff555555));
        ed.onTextChange = [this, &ed]() { updateData(); };
    }

    void updateData()
    {
        song.title = titleEd.getText();
        song.bpm = bpmEd.getText().getDoubleValue();
        if (song.bpm < 40) song.bpm = 40;
        song.ts = tsEd.getText().getIntValue();
        if (song.ts < 1) song.ts = 4;

        switch (colorPicker.getSelectedId()) {
            case 1: song.color = juce::Colour(0xffff7b00); break;
            case 2: song.color = juce::Colour(0xff00a8ff); break;
            case 3: song.color = juce::Colour(0xff4cd137); break;
            case 4: song.color = juce::Colour(0xffff4757); break;
            case 5: song.color = juce::Colour(0xff9c88ff); break;
            case 6: song.color = juce::Colour(0xfffbc531); break;
            default: song.color = juce::Colour(0xffff7b00); break;
        }

        if (onDataChanged) onDataChanged();
    }
};

//==============================================================================
// 4. Main Application Application
//==============================================================================
// Note: We now inherit from juce::ChangeListener to track Audio Device routing!
class MainComponent : public juce::AudioAppComponent, public juce::Timer, public juce::ChangeListener
{
public:
    MainComponent()
    {
        juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypefaceName("Roboto");
        setSize(900, 600);

        // --- Setup Application Storage ---
        juce::PropertiesFile::Options storageOptions;
        storageOptions.applicationName     = "Setlist Metronome";
        storageOptions.folderName          = "SetlistMetronome";
        storageOptions.filenameSuffix      = ".xml";
        storageOptions.osxLibrarySubFolder = "Application Support";
        appProperties.setStorageParameters(storageOptions);

        auto* settings = appProperties.getUserSettings();

        // 1. Load Saved Setlist
        std::unique_ptr<juce::XmlElement> savedData(settings->getXmlValue("Setlist"));
        if (savedData != nullptr && savedData->hasTagName("SETLIST"))
        {
            for (auto* child : savedData->getChildIterator())
            {
                if (child->hasTagName("SONG"))
                    setlist.push_back(Song::fromXml(child));
            }
        }
        if (setlist.empty())
        {
            setlist.push_back({"Opener", 120.0, 4, "Key: G\n\nIntro: G | C | Em | D\n\nVerse 1:\nKeep it driving on the floor tom.", juce::Colour(0xffff7b00)});
            setlist.push_back({"Worship", 72.0, 4, "Key: D\n\nChorus:\nBig crash on downbeat. \nBuild dynamically into Bridge.", juce::Colour(0xff00a8ff)});
            triggerSave();
        }

        // 2. Load Saved Audio Interface / USB Codec
        std::unique_ptr<juce::XmlElement> audioState(settings->getXmlValue("AudioDeviceState"));

        // --- Start Audio Engine using saved settings! ---
        setAudioChannels(0, 2, audioState.get());
        deviceManager.addChangeListener(this); // Listen for when user changes USB Codec

        // --- Top Navigation Bar ---
        navStageBtn.setButtonText("STAGE VIEW");
        navEditBtn.setButtonText("EDIT SETLIST");
        navSettingsBtn.setButtonText("SETTINGS"); // New Settings Tab

        navStageBtn.onClick = [this] { switchView(0); };
        navEditBtn.onClick = [this] { switchView(1); };
        navSettingsBtn.onClick = [this] { switchView(2); };

        addAndMakeVisible(navStageBtn);
        addAndMakeVisible(navEditBtn);
        addAndMakeVisible(navSettingsBtn);

        // --- Stage View Components ---
        addAndMakeVisible(stageViewport);
        stageViewport.setViewedComponent(&stageListContainer, false);
        stageViewport.setScrollBarsShown(true, false);

        notesDisplay.setMultiLine(true);
        notesDisplay.setReturnKeyStartsNewLine(true);
        notesDisplay.setReadOnly(false);
        notesDisplay.setCaretVisible(true);
        notesDisplay.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2d2d2d));
        notesDisplay.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
        notesDisplay.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xff3e3e3e));
        notesDisplay.setFont(juce::Font(22.0f));
        addAndMakeVisible(notesDisplay);

        notesDisplay.onTextChange = [this] {
            if (activeSongIndex >= 0 && activeSongIndex < setlist.size()) {
                setlist[activeSongIndex].notes = notesDisplay.getText();
                triggerSave();
            }
        };

        // --- Editor View Components ---
        addAndMakeVisible(editViewport);
        editViewport.setViewedComponent(&editListContainer, false);
        editViewport.setScrollBarsShown(true, false);

        addSongBtn.setButtonText("+ ADD SONG");
        addSongBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3e3e3e));
        addSongBtn.setColour(juce::TextButton::textColourOnId, juce::Colour(0xffff7b00));
        addSongBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffff7b00));
        addSongBtn.onClick = [this] {
            setlist.push_back({"New Song", 120.0, 4, "Notes here...", juce::Colour(0xffff7b00)});
            triggerSave();
            rebuildEditor();
        };
        addChildComponent(addSongBtn);

        // --- Settings View Components (Audio Routing) ---
        audioSetupComp = std::make_unique<juce::AudioDeviceSelectorComponent>(
            deviceManager,
            0, 0, // min/max inputs (we don't need mics)
            0, 2, // min/max outputs (stereo out)
            false, false, false, false); // No midi, simple view
        addChildComponent(audioSetupComp.get());

        activeSongIndex = 0;
        if (!setlist.empty())
            notesDisplay.setText(setlist[0].notes, juce::dontSendNotification);

        switchView(0); // Default to stage view
        startTimerHz(30);
    }

    ~MainComponent() override {
        deviceManager.removeChangeListener(this);
        if (pendingSave) saveSetlistToDisk();
        shutdownAudio();
    }

    // --- Audio Routing Callback ---
    // If you select your USB Codec in the UI, this saves it to your hard drive permanently
    void changeListenerCallback(juce::ChangeBroadcaster* source) override
    {
        if (source == &deviceManager)
        {
            if (auto xml = deviceManager.createStateXml())
            {
                if (auto* settings = appProperties.getUserSettings())
                {
                    settings->setValue("AudioDeviceState", xml.get());
                    settings->saveIfNeeded();
                }
            }
        }
    }

    //==============================================================================
    // AUDIO THREAD
    //==============================================================================
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        clickSynth.prepare(sampleRate);
        currentSampleRate = sampleRate;
        samplesSinceLastClick = 9999999;
        currentBeat = 0;
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();

        if (!atomicIsPlaying.load()) return;

        double currentBpm = atomicBpm.load();
        int currentTs = atomicTimeSignature.load();
        auto samplesPerBeat = static_cast<int>((60.0 / currentBpm) * currentSampleRate);

        auto* leftChannel = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
        auto* rightChannel = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

        for (int i = 0; i < bufferToFill.numSamples; ++i)
        {
            if (samplesSinceLastClick >= samplesPerBeat)
            {
                bool isDownbeat = (currentBeat == 0);
                clickSynth.trigger(isDownbeat);
                uiPulseFlag.store(isDownbeat ? 2 : 1);
                samplesSinceLastClick = 0;
                currentBeat = (currentBeat + 1) % currentTs;
            }

            float sample = clickSynth.getNextSample();
            leftChannel[i] = sample;
            rightChannel[i] = sample;
            samplesSinceLastClick++;
        }
    }

    void releaseResources() override {}

    //==============================================================================
    // UI THREAD
    //==============================================================================
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff1e1e1e));
        g.setColour(juce::Colour(0xff1e1e1e));
        g.fillRect(0, 0, getWidth(), 50);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        auto navArea = bounds.removeFromTop(50).reduced(10, 5);
        int navWidth = 150;
        navStageBtn.setBounds(navArea.removeFromLeft(navWidth));
        navArea.removeFromLeft(10);
        navEditBtn.setBounds(navArea.removeFromLeft(navWidth));
        navArea.removeFromLeft(10);
        navSettingsBtn.setBounds(navArea.removeFromLeft(navWidth));

        bounds.reduce(20, 10);

        if (currentView == 0) // Stage View
        {
            auto leftHalf = bounds.removeFromLeft(bounds.getWidth() / 2 - 10);
            stageViewport.setBounds(leftHalf);
            notesDisplay.setBounds(bounds.removeFromRight(bounds.getWidth()));

            int y = 0;
            for (auto* tile : stageTiles)
            {
                tile->setBounds(0, y, leftHalf.getWidth() - 20, 90);
                y += 100;
            }
            stageListContainer.setBounds(0, 0, leftHalf.getWidth() - 20, y);
        }
        else if (currentView == 1) // Editor View
        {
            addSongBtn.setBounds(bounds.removeFromTop(40).removeFromLeft(200));
            bounds.removeFromTop(10);
            editViewport.setBounds(bounds);

            int y = 0;
            for (auto* row : editorRows)
            {
                row->setBounds(0, y, bounds.getWidth() - 20, 40);
                y += 45;
            }
            editListContainer.setBounds(0, 0, bounds.getWidth() - 20, y);
        }
        else if (currentView == 2) // Settings View
        {
            // Center the audio setup box
            audioSetupComp->setBounds(bounds.withSizeKeepingCentre(600, 400));
        }
    }

    void timerCallback() override
    {
        int flag = uiPulseFlag.exchange(0);
        if (flag > 0) visualPulseIntensity = 1.0f;

        if (visualPulseIntensity > 0.0f)
        {
            visualPulseIntensity -= 0.1f;
            if (activeSongIndex >= 0 && activeSongIndex < stageTiles.size())
            {
                stageTiles[activeSongIndex]->setPulse(visualPulseIntensity);
            }
        }

        if (pendingSave)
        {
            saveCountdown--;
            if (saveCountdown <= 0)
            {
                saveSetlistToDisk();
                pendingSave = false;
            }
        }
    }

private:
    void triggerSave()
    {
        pendingSave = true;
        saveCountdown = 30;
    }

    void saveSetlistToDisk()
    {
        juce::XmlElement xml("SETLIST");
        for (const auto& song : setlist)
            xml.addChildElement(song.toXml());

        if (auto* settings = appProperties.getUserSettings())
        {
            settings->setValue("Setlist", &xml);
            settings->saveIfNeeded();
        }
    }

    void styleNavBtn(juce::TextButton& btn, bool isActive)
    {
        btn.setColour(juce::TextButton::buttonColourId, isActive ? juce::Colour(0xff3e3e3e) : juce::Colours::transparentBlack);
        btn.setColour(juce::TextButton::textColourOnId, isActive ? juce::Colour(0xffff7b00) : juce::Colours::grey);
        btn.setColour(juce::TextButton::textColourOffId, isActive ? juce::Colour(0xffff7b00) : juce::Colours::grey);
    }

    void switchView(int viewIndex)
    {
        currentView = viewIndex;
        styleNavBtn(navStageBtn, currentView == 0);
        styleNavBtn(navEditBtn, currentView == 1);
        styleNavBtn(navSettingsBtn, currentView == 2);

        stageViewport.setVisible(currentView == 0);
        notesDisplay.setVisible(currentView == 0);

        editViewport.setVisible(currentView == 1);
        addSongBtn.setVisible(currentView == 1);

        audioSetupComp->setVisible(currentView == 2);

        if (currentView == 0) rebuildStage();
        else if (currentView == 1) rebuildEditor();

        resized();
    }

    void rebuildStage()
    {
        stageTiles.clear();
        for (int i = 0; i < setlist.size(); ++i)
        {
            auto* tile = new StageTile(setlist[i], i, [this](int idx) { handleTileClick(idx); });
            stageListContainer.addAndMakeVisible(tile);
            stageTiles.add(tile);

            if (i == activeSongIndex && atomicIsPlaying.load()) tile->setActive(true);
        }

        if (activeSongIndex >= 0 && activeSongIndex < setlist.size())
            notesDisplay.setText(setlist[activeSongIndex].notes, juce::dontSendNotification);

        resized();
    }

    void rebuildEditor()
    {
        editorRows.clear();
        for (int i = 0; i < setlist.size(); ++i)
        {
            auto* row = new EditorRow(setlist[i],
            [this, i]() {
                setlist.erase(setlist.begin() + i);
                if (activeSongIndex == i) {
                    atomicIsPlaying.store(false);
                    activeSongIndex = 0;
                    if (!setlist.empty())
                        notesDisplay.setText(setlist[0].notes, juce::dontSendNotification);
                }
                triggerSave();
                rebuildEditor();
            },
            [this]() {
                triggerSave();
            });

            editListContainer.addAndMakeVisible(row);
            editorRows.add(row);
        }
        resized();
    }

    void handleTileClick(int index)
    {
        if (atomicIsPlaying.load() && activeSongIndex == index)
        {
            atomicIsPlaying.store(false);
            if (activeSongIndex >= 0 && activeSongIndex < stageTiles.size())
                stageTiles[activeSongIndex]->setActive(false);
            return;
        }

        if (activeSongIndex >= 0 && activeSongIndex < stageTiles.size())
            stageTiles[activeSongIndex]->setActive(false);

        activeSongIndex = index;
        stageTiles[index]->setActive(true);

        notesDisplay.setText(setlist[index].notes, juce::dontSendNotification);

        atomicBpm.store(setlist[index].bpm);
        atomicTimeSignature.store(setlist[index].ts);

        samplesSinceLastClick = 9999999;
        currentBeat = 0;
        atomicIsPlaying.store(true);
    }

    std::vector<Song> setlist;
    int currentView = 0; // 0=Stage, 1=Edit, 2=Settings
    int activeSongIndex = -1;

    juce::ApplicationProperties appProperties;
    bool pendingSave = false;
    int saveCountdown = 0;

    juce::TextButton navStageBtn, navEditBtn, navSettingsBtn;
    std::unique_ptr<juce::AudioDeviceSelectorComponent> audioSetupComp;

    juce::Viewport stageViewport;
    juce::Component stageListContainer;
    juce::OwnedArray<StageTile> stageTiles;
    juce::TextEditor notesDisplay;

    juce::Viewport editViewport;
    juce::Component editListContainer;
    juce::OwnedArray<EditorRow> editorRows;
    juce::TextButton addSongBtn;

    WorshipClickSynth clickSynth;
    double currentSampleRate = 44100.0;
    int samplesSinceLastClick = 0;
    int currentBeat = 0;

    std::atomic<bool> atomicIsPlaying { false };
    std::atomic<double> atomicBpm { 120.0 };
    std::atomic<int> atomicTimeSignature { 4 };
    std::atomic<int> uiPulseFlag { 0 };
    float visualPulseIntensity = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};