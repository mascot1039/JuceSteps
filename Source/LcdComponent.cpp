#include "LcdComponent.h"
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"

// ==========================================
// LcdComponent の実装
// ==========================================
LcdComponent::LcdComponent() {
    // 液晶画面の初期サイズ（必要に応じて変更してください）
    //setSize(600, 800);
}

void LcdComponent::paint(juce::Graphics& g) {
    // 1. 背景色
    g.fillAll(juce::Colour::fromRGB(15, 30, 20));

    // 2. 枠線
    g.setColour(juce::Colours::black);
    g.drawRect(getLocalBounds(), 3);

    // 3. 文字の設定（等幅フォント）
    float fontSize = 24.0f;
    juce::Font font(juce::Font::getDefaultMonospacedFontName(), fontSize, juce::Font::plain);
    g.setFont(font);

    // 4. 文字列の組み立て
    // フォーマット：「000-00-000 S:096 N:C 3(060) V:064 G:0096」
    juce::String noteNumber = juce::String::formatted("%03d", mNoteNumber);
    if (mNoteNumber == -1)
    {
        noteNumber = "---";
    }
    juce::String text = juce::String::formatted(
        "%03d-%02d-%03d S:%03d N:%-4.4s(%3.3s) V:%03d G:%04d",
        mMeasure, mBeat, mClock,
        mStepTime,
        mNoteName.toRawUTF8(), noteNumber.toRawUTF8(),
        mVelocity,
        mGateTime
    );

    // 🌟 描画位置の基準を明確にする
    // reduced(18) と同じ左上座標を取得
    auto textBounds = getLocalBounds().reduced(18);
    float startX = static_cast<float>(textBounds.getX());
    float startY = static_cast<float>(textBounds.getY());

    // フォントから1文字の正確な横幅（アドバンス幅）を取得する
    float charWidth = font.getStringWidthFloat ("A");
    float fontHeight = font.getHeight();

    // 5. 選択項目のハイライト（反転四角形）の座標計算
    float highlightX = startX;
    float highlightW = 0.0f;
    float highlightY = startY - 2.0f; // 文字の高さに合わせる微調整
    float highlightH = fontHeight + 4.0f;

    if (mCurrentMode != EditMode::None)
    {
        switch (mCurrentMode)
        {
            case EditMode::None:
                break;
            case EditMode::StepTime:
                highlightX += charWidth * 13.0f; // StepTime「0096」の開始文字位置
                highlightW = charWidth * 3.0f;   // 3文字分
                break;
            case EditMode::Note:
                highlightX += charWidth * 19.0f; // Note「C#-1(060)」の開始文字位置
                highlightW = charWidth * 9.0f;   // 9文字分
                break;
            case EditMode::Velocity:
                highlightX += charWidth * 31.0f; // Velocity「064」の開始文字位置
                highlightW = charWidth * 3.0f;   // 3文字分
                break;
            case EditMode::GateTime:
                highlightX += charWidth * 37.0f; // GateTime「0096」の開始文字位置
                highlightW = charWidth * 4.0f;   // 4文字分
                break;
            default:
                break;
        }

        // 先に背景に黄緑色の四角形を描画
        if (highlightW > 0.0f)
        {
            g.setColour(juce::Colour::fromRGB(50, 220, 100));
            g.fillRect(highlightX, highlightY, highlightW, highlightH);
        }
    }

    // 6. 通常の文字を描画（ベースとなる鮮やかな黄緑色）
    g.setColour(juce::Colour::fromRGB(50, 220, 100));
    g.drawText(text, textBounds, juce::Justification::topLeft, true);

    // 7. 【重要・修正】反転部分の中央の文字だけを「黒（暗いグレー）」でくり抜く処理
    if (mCurrentMode != EditMode::None && highlightW > 0.0f)
    {
        g.saveState();

        // 🌟 グラフィックスの描画エリアを「黄緑色の四角形の中だけ」に制限する（これで他が暗くなりません）
        g.reduceClipRegion (juce::Rectangle<float> (highlightX, highlightY, highlightW, highlightH).getSmallestIntegerContainer());

        // 文字列の色を「背景の暗いグレー」にして、制限エリア内（四角形の中）だけを上書き描画
        g.setColour(juce::Colour::fromRGB(15, 30, 20));
        g.drawText(text, textBounds, juce::Justification::topLeft, true);

        g.restoreState();
    }
}

// 外部から値を変更するための関数（値が変わったら repaint() で画面を再描画する）
void LcdComponent::setTimePosition(int measure, int beat, int clock) {
    mMeasure = measure; mBeat = beat; mClock = clock;
    repaint();
}
void LcdComponent::setNoteInfo(const juce::String& name, int number) {
    mNoteName = name; mNoteNumber = number;
    repaint();
}
void LcdComponent::setMeasure(int measure)
{
    mMeasure = measure;
    repaint();
}
void LcdComponent::setBeat(int beat)
{
    mBeat = beat;
    repaint();
}
void LcdComponent::setClock(int clock)
{
    mClock = clock;
    repaint();
}
void LcdComponent::setStepTime(int step) {
    mStepTime = step;
    repaint();
}
void LcdComponent::setNoteName(const juce::String& name)
{
    mNoteName = name;
    mNoteNumber = getNoteNumberFromName(mNoteName);
    repaint();
    //std::cout << getNoteNumberFromName(name) << std::endl;
}
void LcdComponent::setNoteNumber(int number)
{
    juce::String name = getMc500StyleNoteName(number);
    setNoteInfo(name, number);
}
void LcdComponent::setVelocity(int vel) {
    mVelocity = vel;
    repaint();
}
void LcdComponent::setGateTime(int gate) {
    mGateTime = gate;
    repaint();
}
void LcdComponent::setEditMode (EditMode mode) {
    mCurrentMode = mode;
    repaint(); // モードが変わったら液晶を再描画して反転表示を更新
}

// JUCEの関数を使ってノート名を取得する
juce::String LcdComponent::getMc500StyleNoteName(int noteNumber)
{
    if (noteNumber < 0 || noteNumber > 127)
        return "---";

    // 第1引数: MIDIノート番号 (0-127)
    // 第2引数: true = シャープ表記 (C#), false = フラット表記 (Db)
    // 第3引数: true = オクターブ番号を含める
    // 第4引数: 4 = 中央のC(60)を「C4」とする (これによってノート0が「-1」になります)
    juce::String noteName = juce::MidiMessage::getMidiNoteName(noteNumber, true, true, 4);

    // 【補足】JUCEの標準出力は半角スペースが挟まれない（例: "C-1", "C4", "C#4"）ため、
    // もしMC-500のように「C 4」とシャープ無しの位置を空けたい場合は、文字列を少し加工します。
    if (noteName.length() >= 2 && noteName[1] != '#')
    {
        // 2文字目がシャープ（#）でもマイナス（-）でもない、または「C4」のように2文字目がオクターブ数値の場合
        // 1文字目の後ろに半角スペースを挿入して桁を揃える
        if (noteName[1] != '-') {
            noteName = noteName.substring(0, 1) + " " + noteName.substring(1);
        }
    }
    // オクターブがマイナスで、シャープがない場合（例: "C-1" -> "C -1"）
    if (noteName.startsWith("-") == false && noteName.contains("-") && !noteName.contains("#"))
    {
         noteName = noteName.replace("-", " -");
    }

    return noteName;
}

/**
 * 仕様に完全準拠したノート名からMIDIノート番号を取得する関数
 * @param inputName 入力文字列 (例: "C 4", "Bb5", "C#-1", "G -1")
 * @return MIDIノート番号 (0〜127), 不正な入力・範囲外の場合は -1
 */
int LcdComponent::getNoteNumberFromName(const juce::String& inputName)
{
    // 1. 文字列全体の長さをチェック (3文字、または-1のときの4文字のみ)
    int len = inputName.length();
    if (len != 3 && len != 4) return -1;

    // 大文字化して扱いやすくする
    juce::String src = inputName.toUpperCase();

    // 2. 各文字の抽出（固定位置）
    char noteChar = src[0]; // 1文字目: 音名 (A〜G)
    char accChar  = src[1]; // 2文字目: 記号 (#, B, または半角スペース)

    // 3. バリデーション（不正な文字やフォーマットの拒否）
    if (noteChar < 'A' || noteChar > 'G') return -1;
    if (accChar != '#' && accChar != 'B' && accChar != ' ') return -1;

    // オクターブ指定部分の切り出し
    juce::String octaveStr = src.substring(2);

    // オクターブのバリデーション（3文字なら "0"〜"9"、4文字なら必ず "-1"）
    if (len == 3 && (octaveStr < "0" || octaveStr > "9")) return -1;
    if (len == 4 && octaveStr != "-1") return -1;

    int octave = octaveStr.getIntValue();

    // 4. 音名 (A〜G) の基準半音数 (C=0基準)
    int baseSemitone = 0;
    if      (noteChar == 'C') baseSemitone = 0;
    else if (noteChar == 'D') baseSemitone = 2;
    else if (noteChar == 'E') baseSemitone = 4;
    else if (noteChar == 'F') baseSemitone = 5;
    else if (noteChar == 'G') baseSemitone = 7;
    else if (noteChar == 'A') baseSemitone = 9;
    else if (noteChar == 'B') baseSemitone = 11;

    // 5. 変化記号の計算
    int modifier = 0;
    if (accChar == '#')      modifier = 1;
    else if (accChar == 'B') modifier = -1; // 大文字化により 'b' は 'B' になる

    int totalSemitone = baseSemitone + modifier;

    // 異名同音によるオクターブの繰り上がり・繰り下がり補正 (例: Cb4 -> B3 / B#3 -> C4)
    if (totalSemitone < 0)   { totalSemitone += 12; octave -= 1; }
    if (totalSemitone >= 12) { totalSemitone -= 12; octave += 1; }

    // 補正後のオクターブが仕様（-1〜9）に収まっているかチェック
    if (octave < -1 || octave > 9) return -1;

    // 6. MIDIノート番号の計算 (中央C "C 4" = 60基準)
    int noteNumber = (octave + 1) * 12 + totalSemitone;

    // MIDI規格の絶対範囲 (0〜127) に収まっているかチェック
    if (noteNumber >= 0 && noteNumber <= 127)
    {
        return noteNumber;
    }

    return -1;
}
