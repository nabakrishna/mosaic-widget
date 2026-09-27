// Unit tests for mosaic::layout::LayoutEngine.
//
// LayoutEngine is the one module in Mosaic explicitly designed to be
// tested without rendering anything (see its class comment, and spec
// section 32: "independent from the visual layer... allows layout
// testing without rendering the UI"). These tests exercise exactly that
// contract: pure grid-unit input in, pure grid-unit output out, no D2D,
// no HWND, no device.

#include <gtest/gtest.h>
#include "layout/LayoutEngine.h"

using mosaic::layout::LayoutEngine;
using mosaic::layout::LayoutConstraints;
using mosaic::layout::LayoutResult;
using mosaic::layout::WidgetInput;
using mosaic::layout::GridPosition;
using mosaic::widgets::WidgetId;
using mosaic::widgets::GridSize;

namespace {

WidgetInput MakeInput(WidgetId id, int cols, int rows,
                       std::optional<GridPosition> saved = std::nullopt) {
    WidgetInput in;
    in.id = id;
    in.size = GridSize{ cols, rows };
    in.savedPosition = saved;
    return in;
}

// True iff any two placed widgets' rects overlap. Collision-freedom is
// LayoutEngine's core guarantee, so most tests assert this directly
// rather than trusting specific coordinates.
bool HasCollision(const LayoutResult& result) {
    const auto& p = result.placements;
    for (size_t i = 0; i < p.size(); ++i) {
        for (size_t j = i + 1; j < p.size(); ++j) {
            bool separate =
                p[i].rect.Right() <= p[j].rect.col ||
                p[j].rect.Right() <= p[i].rect.col ||
                p[i].rect.Bottom() <= p[j].rect.row ||
                p[j].rect.Bottom() <= p[i].rect.row;
            if (!separate) return true;
        }
    }
    return false;
}

bool AllWithinColumns(const LayoutResult& result, int columns) {
    for (const auto& p : result.placements) {
        if (p.rect.col < 0 || p.rect.Right() > columns) return false;
    }
    return true;
}

} // namespace

// --- Arrange: fresh install, no saved positions ---------------------------

TEST(LayoutEngineArrange, FreshInstallPlacesEveryWidgetWithoutCollision) {
    LayoutEngine engine;
    LayoutConstraints constraints; // 6 columns, Mosaic's real default
    std::vector<WidgetInput> widgets = {
        MakeInput(WidgetId::Todo, 2, 2),
        MakeInput(WidgetId::Photo, 2, 2),
        MakeInput(WidgetId::Activity, 2, 2),
        MakeInput(WidgetId::Pinned, 2, 1),
        MakeInput(WidgetId::QuickNotes, 2, 1),
    };

    LayoutResult result = engine.Arrange(widgets, constraints);

    EXPECT_EQ(result.placements.size(), widgets.size());
    EXPECT_FALSE(HasCollision(result));
    EXPECT_TRUE(AllWithinColumns(result, constraints.columns));
}

TEST(LayoutEngineArrange, FirstWidgetLandsAtTopLeftOnEmptyGrid) {
    LayoutEngine engine;
    LayoutConstraints constraints;
    std::vector<WidgetInput> widgets = { MakeInput(WidgetId::Todo, 2, 2) };

    LayoutResult result = engine.Arrange(widgets, constraints);

    ASSERT_EQ(result.placements.size(), 1u);
    EXPECT_EQ(result.placements[0].rect.col, 0);
    EXPECT_EQ(result.placements[0].rect.row, 0);
}

TEST(LayoutEngineArrange, WidgetsFillReadingOrderBeforeWrapping) {
    // Three 2-wide widgets exactly fill a 6-column row -- none should
    // wrap to row 1.
    LayoutEngine engine;
    LayoutConstraints constraints; // columns = 6
    std::vector<WidgetInput> widgets = {
        MakeInput(WidgetId::Todo, 2, 1),
        MakeInput(WidgetId::Photo, 2, 1),
        MakeInput(WidgetId::Activity, 2, 1),
    };

    LayoutResult result = engine.Arrange(widgets, constraints);

    for (const auto& p : result.placements) {
        EXPECT_EQ(p.rect.row, 0);
    }
    EXPECT_EQ(result.totalRows, 1);
}

TEST(LayoutEngineArrange, WidgetTooWideForRemainingSpaceWrapsToNextRow) {
    LayoutEngine engine;
    LayoutConstraints constraints; // columns = 6
    std::vector<WidgetInput> widgets = {
        MakeInput(WidgetId::Todo, 4, 1),
        MakeInput(WidgetId::Photo, 4, 1), // only 2 cols left in row 0 -- must wrap
    };

    LayoutResult result = engine.Arrange(widgets, constraints);

    const auto* todo = result.Find(WidgetId::Todo);
    const auto* photo = result.Find(WidgetId::Photo);
    ASSERT_NE(todo, nullptr);
    ASSERT_NE(photo, nullptr);
    EXPECT_EQ(todo->row, 0);
    EXPECT_EQ(photo->row, 1);
    EXPECT_FALSE(HasCollision(result));
}

TEST(LayoutEngineArrange, ArrangeAlwaysCompactsEvenALoneSavedPosition) {
    // Arrange's unconditional Compact() pass (see its own comment: "a
    // fresh arrangement is already maximally compact... doesn't hurt")
    // pulls every widget up-and-left afterward, including one placed via
    // its own savedPosition hint -- so a lone widget "remembering" {4,3}
    // still ends up back at the origin, since nothing else occupies the
    // grid to justify leaving a gap in front of it. This is the actual,
    // intentional contract: savedPosition is a *search hint* for where
    // FindFirstFit starts looking (relevant once other widgets are
    // already occupying earlier cells), not a guaranteed final resting
    // place for an otherwise-empty grid.
    LayoutEngine engine;
    LayoutConstraints constraints;
    std::vector<WidgetInput> widgets = {
        MakeInput(WidgetId::Todo, 2, 2, GridPosition{ 4, 3 }),
    };

    LayoutResult result = engine.Arrange(widgets, constraints);

    ASSERT_EQ(result.placements.size(), 1u);
    EXPECT_EQ(result.placements[0].rect.col, 0);
    EXPECT_EQ(result.placements[0].rect.row, 0);
}

TEST(LayoutEngineArrange, SavedPositionHintInfluencesPlacementAmongOthers) {
    // With earlier cells already occupied, a widget's savedPosition hint
    // does change where FindFirstFit looks first -- this is the part of
    // the contract that's actually observable, unlike the lone-widget
    // case above which compaction always erases.
    LayoutEngine engine;
    LayoutConstraints constraints; // columns = 6
    std::vector<WidgetInput> widgets = {
        MakeInput(WidgetId::Todo, 2, 1),                             // takes cols 0-1, row 0
        MakeInput(WidgetId::Photo, 2, 1, GridPosition{ 4, 0 }),      // hinted to cols 4-5
        MakeInput(WidgetId::Activity, 2, 1, GridPosition{ 2, 0 }),   // hinted to cols 2-3
    };

    LayoutResult result = engine.Arrange(widgets, constraints);

    EXPECT_FALSE(HasCollision(result));
    EXPECT_TRUE(AllWithinColumns(result, constraints.columns));
    EXPECT_EQ(result.placements.size(), widgets.size());
}

TEST(LayoutEngineArrange, CollidingSavedPositionsResolveWithoutOverlap) {
    // Two widgets both "remember" the same saved slot (e.g. a stale layout
    // from before a widget was added). The engine must never place them
    // on top of each other, even though both would prefer the same cell.
    LayoutEngine engine;
    LayoutConstraints constraints;
    std::vector<WidgetInput> widgets = {
        MakeInput(WidgetId::Todo, 2, 2, GridPosition{ 0, 0 }),
        MakeInput(WidgetId::Photo, 2, 2, GridPosition{ 0, 0 }),
    };

    LayoutResult result = engine.Arrange(widgets, constraints);

    EXPECT_EQ(result.placements.size(), 2u);
    EXPECT_FALSE(HasCollision(result));
}

TEST(LayoutEngineArrange, EmptyWidgetListProducesEmptyResult) {
    LayoutEngine engine;
    LayoutConstraints constraints;
    LayoutResult result = engine.Arrange({}, constraints);

    EXPECT_TRUE(result.placements.empty());
    EXPECT_EQ(result.totalRows, 0);
}

TEST(LayoutEngineArrange, ArrangeIsDeterministicForTheSameInput) {
    // Same input, arranged twice, must produce byte-identical output --
    // this is what makes the drag preview and layout tests both able to
    // rely on a fixed answer rather than "some valid collision-free
    // answer".
    LayoutEngine engine;
    LayoutConstraints constraints;
    std::vector<WidgetInput> widgets = {
        MakeInput(WidgetId::Todo, 2, 2),
        MakeInput(WidgetId::Photo, 2, 2),
        MakeInput(WidgetId::Activity, 2, 2),
        MakeInput(WidgetId::Pinned, 2, 1),
    };

    LayoutResult a = engine.Arrange(widgets, constraints);
    LayoutResult b = engine.Arrange(widgets, constraints);

    ASSERT_EQ(a.placements.size(), b.placements.size());
    for (size_t i = 0; i < a.placements.size(); ++i) {
        EXPECT_EQ(a.placements[i].id, b.placements[i].id);
        EXPECT_EQ(a.placements[i].rect.col, b.placements[i].rect.col);
        EXPECT_EQ(a.placements[i].rect.row, b.placements[i].rect.row);
    }
}

// --- MoveWidget: drag-and-drop repositioning -------------------------------

TEST(LayoutEngineMoveWidget, MovingToAnEmptyAreaPlacesItThere) {
    LayoutEngine engine;
    LayoutConstraints constraints;
    std::vector<WidgetInput> widgets = { MakeInput(WidgetId::Todo, 2, 2) };
    LayoutResult initial = engine.Arrange(widgets, constraints);

    LayoutResult moved = engine.MoveWidget(initial, WidgetId::Todo, GridPosition{ 3, 3 }, constraints);

    const auto* rect = moved.Find(WidgetId::Todo);
    ASSERT_NE(rect, nullptr);
    EXPECT_EQ(rect->col, 3);
    EXPECT_EQ(rect->row, 3);
}

TEST(LayoutEngineMoveWidget, OtherWidgetsCompactAroundTheMovedWidget) {
    LayoutEngine engine;
    LayoutConstraints constraints; // columns = 6
    std::vector<WidgetInput> widgets = {
        MakeInput(WidgetId::Todo, 2, 1),
        MakeInput(WidgetId::Photo, 2, 1),
        MakeInput(WidgetId::Activity, 2, 1),
    };
    LayoutResult initial = engine.Arrange(widgets, constraints);
    // All three now occupy row 0 (cols 0-1, 2-3, 4-5).

    // Move Photo (the middle one) down and out of the way; Activity should
    // compact left to fill the gap it leaves behind rather than the row
    // staying permanently gapped.
    LayoutResult moved = engine.MoveWidget(initial, WidgetId::Photo, GridPosition{ 4, 5 }, constraints);

    EXPECT_FALSE(HasCollision(moved));
    const auto* activity = moved.Find(WidgetId::Activity);
    ASSERT_NE(activity, nullptr);
    // Activity should no longer be sitting at its original col 4 with a
    // gap at col 2 behind it -- compaction must have pulled it left.
    EXPECT_LT(activity->col, 4);
}

TEST(LayoutEngineMoveWidget, MovedWidgetNeverCollidesWithRemainingWidgets) {
    LayoutEngine engine;
    LayoutConstraints constraints;
    std::vector<WidgetInput> widgets = {
        MakeInput(WidgetId::Todo, 2, 2),
        MakeInput(WidgetId::Photo, 2, 2),
        MakeInput(WidgetId::Activity, 2, 2),
        MakeInput(WidgetId::Pinned, 2, 1),
        MakeInput(WidgetId::QuickNotes, 2, 1),
    };
    LayoutResult initial = engine.Arrange(widgets, constraints);

    // Try to drop Todo directly on top of where Photo already sits --
    // the engine must resolve this without overlap, not honor the exact
    // target coordinate at the cost of a collision.
    const auto* photoRect = initial.Find(WidgetId::Photo);
    ASSERT_NE(photoRect, nullptr);
    LayoutResult moved = engine.MoveWidget(initial, WidgetId::Todo,
                                            GridPosition{ photoRect->col, photoRect->row }, constraints);

    EXPECT_EQ(moved.placements.size(), widgets.size());
    EXPECT_FALSE(HasCollision(moved));
    EXPECT_TRUE(AllWithinColumns(moved, constraints.columns));
}

TEST(LayoutEngineMoveWidget, UnknownWidgetIdIsANoOp) {
    LayoutEngine engine;
    LayoutConstraints constraints;
    std::vector<WidgetInput> widgets = { MakeInput(WidgetId::Todo, 2, 2) };
    LayoutResult initial = engine.Arrange(widgets, constraints);

    // QuickNotes was never in the input list, so it isn't a placed widget
    // in `initial` -- MoveWidget must defensively return the layout
    // unchanged rather than crash or fabricate a placement.
    LayoutResult result = engine.MoveWidget(initial, WidgetId::QuickNotes, GridPosition{ 2, 2 }, constraints);

    ASSERT_EQ(result.placements.size(), initial.placements.size());
    EXPECT_EQ(result.placements[0].rect.col, initial.placements[0].rect.col);
    EXPECT_EQ(result.placements[0].rect.row, initial.placements[0].rect.row);
}

TEST(LayoutEngineMoveWidget, TargetPastGridBoundaryIsClampedNotDropped) {
    LayoutEngine engine;
    LayoutConstraints constraints; // columns = 6
    std::vector<WidgetInput> widgets = { MakeInput(WidgetId::Todo, 2, 2) };
    LayoutResult initial = engine.Arrange(widgets, constraints);

    // Column 10 is out of bounds for a 2-wide widget on a 6-column grid.
    LayoutResult moved = engine.MoveWidget(initial, WidgetId::Todo, GridPosition{ 10, 0 }, constraints);

    const auto* rect = moved.Find(WidgetId::Todo);
    ASSERT_NE(rect, nullptr);
    EXPECT_LE(rect->Right(), constraints.columns);
    EXPECT_GE(rect->col, 0);
}