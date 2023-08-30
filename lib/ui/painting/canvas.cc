// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/canvas.h"

#include <cmath>

#include "flutter/display_list/dl_canvas.h"
#include "flutter/lib/ui/floating_point.h"
#include "flutter/lib/ui/painting/image.h"
#include "flutter/lib/ui/painting/image_filter.h"
#include "flutter/lib/ui/painting/paint.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/lib/ui/window/platform_configuration.h"
#include "flutter/lib/ui/window/window.h"

using tonic::ToDart;

namespace flutter {

IMPLEMENT_WRAPPERTYPEINFO(ui, Canvas);

void Canvas::Create(Dart_Handle wrapper,
                    PictureRecorder* recorder,
                    double left,
                    double top,
                    double right,
                    double bottom) {
  UIDartState::ThrowIfUIOperationsProhibited();

  if (!recorder) {
    Dart_ThrowException(
        ToDart("Canvas constructor called with non-genuine PictureRecorder."));
    return;
  }

  fml::RefPtr<Canvas> canvas =
      fml::MakeRefCounted<Canvas>(recorder->BeginRecording(
          SkRect::MakeLTRB(SafeNarrow(left), SafeNarrow(top), SafeNarrow(right),
                           SafeNarrow(bottom))));
  FML_DCHECK(canvas->dl_canvas_);
  recorder->set_canvas(canvas);
  canvas->AssociateWithDartWrapper(wrapper);
}

Canvas::Canvas(DlCanvas* canvas) : dl_canvas_(canvas) {}

Canvas::~Canvas() {}

void Canvas::save() {
  if (dl_canvas_) {
    dl_canvas_->Save();
  }
}

void Canvas::saveLayerWithoutBounds(Dart_Handle paint_objects,
                                    Dart_Handle paint_data) {
  Paint paint(paint_objects, paint_data);

  FML_DCHECK(paint.isNotNull());
  if (dl_canvas_) {
    DlPaint dl_paint;
    const DlPaint* save_paint = paint.paint(dl_paint, kSaveLayerWithPaintFlags);
    FML_DCHECK(save_paint);
    TRACE_EVENT0("flutter", "ui.Canvas::saveLayer (Recorded)");
    dl_canvas_->SaveLayer(nullptr, save_paint);
  }
}

void Canvas::saveLayer(double left,
                       double top,
                       double right,
                       double bottom,
                       Dart_Handle paint_objects,
                       Dart_Handle paint_data) {
  Paint paint(paint_objects, paint_data);

  FML_DCHECK(paint.isNotNull());
  SkRect bounds = SkRect::MakeLTRB(SafeNarrow(left), SafeNarrow(top),
                                   SafeNarrow(right), SafeNarrow(bottom));
  if (dl_canvas_) {
    DlPaint dl_paint;
    const DlPaint* save_paint = paint.paint(dl_paint, kSaveLayerWithPaintFlags);
    FML_DCHECK(save_paint);
    TRACE_EVENT0("flutter", "ui.Canvas::saveLayer (Recorded)");
    dl_canvas_->SaveLayer(&bounds, save_paint);
  }
}

void Canvas::restore() {
  if (dl_canvas_) {
    dl_canvas_->Restore();
  }
}

int Canvas::getSaveCount() {
  if (dl_canvas_) {
    return dl_canvas_->GetSaveCount();
  } else {
    return 0;
  }
}

void Canvas::restoreToCount(int count) {
  if (dl_canvas_ && count < getSaveCount()) {
    dl_canvas_->RestoreToCount(count);
  }
}

void Canvas::translate(double dx, double dy) {
  if (dl_canvas_) {
    dl_canvas_->Translate(SafeNarrow(dx), SafeNarrow(dy));
  }
}

void Canvas::scale(double sx, double sy) {
  if (dl_canvas_) {
    dl_canvas_->Scale(SafeNarrow(sx), SafeNarrow(sy));
  }
}

void Canvas::rotate(double radians) {
  if (dl_canvas_) {
    dl_canvas_->Rotate(SafeNarrow(radians) * 180.0f / static_cast<float>(M_PI));
  }
}

void Canvas::skew(double sx, double sy) {
  if (dl_canvas_) {
    dl_canvas_->Skew(SafeNarrow(sx), SafeNarrow(sy));
  }
}

void Canvas::transform(const tonic::Float64List& matrix4) {
  // The Float array stored by Dart Matrix4 is in column-major order
  // Both DisplayList and SkM44 constructor take row-major matrix order
  if (dl_canvas_) {
    // clang-format off
    dl_canvas_->TransformFullPerspective(
        SafeNarrow(matrix4[ 0]), SafeNarrow(matrix4[ 4]), SafeNarrow(matrix4[ 8]), SafeNarrow(matrix4[12]),
        SafeNarrow(matrix4[ 1]), SafeNarrow(matrix4[ 5]), SafeNarrow(matrix4[ 9]), SafeNarrow(matrix4[13]),
        SafeNarrow(matrix4[ 2]), SafeNarrow(matrix4[ 6]), SafeNarrow(matrix4[10]), SafeNarrow(matrix4[14]),
        SafeNarrow(matrix4[ 3]), SafeNarrow(matrix4[ 7]), SafeNarrow(matrix4[11]), SafeNarrow(matrix4[15]));
    // clang-format on
  }
}

void Canvas::getTransform(Dart_Handle matrix4_handle) {
  if (dl_canvas_) {
    SkM44 sk_m44 = dl_canvas_->GetTransformFullPerspective();
    SkScalar m44_values[16];
    // The Float array stored by Dart Matrix4 is in column-major order
    sk_m44.getColMajor(m44_values);
    auto matrix4 = tonic::Float64List(matrix4_handle);
    for (int i = 0; i < 16; i++) {
      matrix4[i] = m44_values[i];
    }
  }
}

void Canvas::clipRect(double left,
                      double top,
                      double right,
                      double bottom,
                      DlCanvas::ClipOp clipOp,
                      bool doAntiAlias) {
  if (dl_canvas_) {
    dl_canvas_->ClipRect(
        SkRect::MakeLTRB(SafeNarrow(left), SafeNarrow(top), SafeNarrow(right),
                         SafeNarrow(bottom)),
        clipOp, doAntiAlias);
  }
}

void Canvas::clipRRect(const RRect& rrect, bool doAntiAlias) {
  if (dl_canvas_) {
    dl_canvas_->ClipRRect(rrect.sk_rrect, DlCanvas::ClipOp::kIntersect,
                          doAntiAlias);
  }
}

void Canvas::clipPath(const CanvasPath* path, bool doAntiAlias) {
  if (!path) {
    Dart_ThrowException(
        ToDart("Canvas.clipPath called with non-genuine Path."));
    return;
  }
  if (dl_canvas_) {
    dl_canvas_->ClipPath(path->path(), DlCanvas::ClipOp::kIntersect,
                         doAntiAlias);
  }
}

void Canvas::getDestinationClipBounds(Dart_Handle rect_handle) {
  if (dl_canvas_) {
    auto rect = tonic::Float64List(rect_handle);
    SkRect bounds = dl_canvas_->GetDestinationClipBounds();
    rect[0] = bounds.fLeft;
    rect[1] = bounds.fTop;
    rect[2] = bounds.fRight;
    rect[3] = bounds.fBottom;
  }
}

void Canvas::getLocalClipBounds(Dart_Handle rect_handle) {
  if (dl_canvas_) {
    auto rect = tonic::Float64List(rect_handle);
    SkRect bounds = dl_canvas_->GetLocalClipBounds();
    rect[0] = bounds.fLeft;
    rect[1] = bounds.fTop;
    rect[2] = bounds.fRight;
    rect[3] = bounds.fBottom;
  }
}

void Canvas::drawColor(SkColor color, DlBlendMode blend_mode) {
  if (dl_canvas_) {
    dl_canvas_->DrawColor(color, blend_mode);
  }
}

void Canvas::drawLine(double x1,
                      double y1,
                      double x2,
                      double y2,
                      Dart_Handle paint_objects,
                      Dart_Handle paint_data) {
  Paint paint(paint_objects, paint_data);

  FML_DCHECK(paint.isNotNull());
  if (dl_canvas_) {
    DlPaint dl_paint;
    paint.paint(dl_paint, kDrawLineFlags);
    dl_canvas_->DrawLine(SkPoint::Make(SafeNarrow(x1), SafeNarrow(y1)),
                         SkPoint::Make(SafeNarrow(x2), SafeNarrow(y2)),
                         dl_paint);
  }
}

void Canvas::drawPaint(Dart_Handle paint_objects, Dart_Handle paint_data) {
  Paint paint(paint_objects, paint_data);

  FML_DCHECK(paint.isNotNull());
  if (dl_canvas_) {
    DlPaint dl_paint;
    paint.paint(dl_paint, kDrawPaintFlags);
    std::shared_ptr<const DlImageFilter> filter = dl_paint.getImageFilter();
    if (filter && !filter->asColorFilter()) {
      // drawPaint does an implicit saveLayer if an SkImageFilter is
      // present that cannot be replaced by an SkColorFilter.
      TRACE_EVENT0("flutter", "ui.Canvas::saveLayer (Recorded)");
    }
    dl_canvas_->DrawPaint(dl_paint);
  }
}

void Canvas::drawRect(double left,
                      double top,
                      double right,
                      double bottom,
                      Dart_Handle paint_objects,
                      Dart_Handle paint_data) {
  Paint paint(paint_objects, paint_data);

  FML_DCHECK(paint.isNotNull());
  if (dl_canvas_) {
    DlPaint dl_paint;
    paint.paint(dl_paint, kDrawRectFlags);
    dl_canvas_->DrawRect(
        SkRect::MakeLTRB(SafeNarrow(left), SafeNarrow(top), SafeNarrow(right),
                         SafeNarrow(bottom)),
        dl_paint);
  }
}

void Canvas::drawRRect(const RRect& rrect,
                       Dart_Handle paint_objects,
                       Dart_Handle paint_data) {
  Paint paint(paint_objects, paint_data);

  FML_DCHECK(paint.isNotNull());
  if (dl_canvas_) {
    DlPaint dl_paint;
    paint.paint(dl_paint, kDrawRRectFlags);
    dl_canvas_->DrawRRect(rrect.sk_rrect, dl_paint);
  }
}

void Canvas::drawDRRect(const RRect& outer,
                        const RRect& inner,
                        Dart_Handle paint_objects,
                        Dart_Handle paint_data) {
  Paint paint(paint_objects, paint_data);

  FML_DCHECK(paint.isNotNull());
  if (dl_canvas_) {
    DlPaint dl_paint;
    paint.paint(dl_paint, kDrawDRRectFlags);
    dl_canvas_->DrawDRRect(outer.sk_rrect, inner.sk_rrect, dl_paint);
  }
}

void Canvas::drawOval(double left,
                      double top,
                      double right,
                      double bottom,
                      Dart_Handle paint_objects,
                      Dart_Handle paint_data) {
  Paint paint(paint_objects, paint_data);

  FML_DCHECK(paint.isNotNull());
  if (dl_canvas_) {
    DlPaint dl_paint;
    paint.paint(dl_paint, kDrawOvalFlags);
    dl_canvas_->DrawOval(
        SkRect::MakeLTRB(SafeNarrow(left), SafeNarrow(top), SafeNarrow(right),
                         SafeNarrow(bottom)),
        dl_paint);
  }
}

void Canvas::drawCircle(double x,
                        double y,
                        double radius,
                        Dart_Handle paint_objects,
                        Dart_Handle paint_data) {
  Paint paint(paint_objects, paint_data);

  FML_DCHECK(paint.isNotNull());
  if (dl_canvas_) {
    DlPaint dl_paint;
    paint.paint(dl_paint, kDrawCircleFlags);
    dl_canvas_->DrawCircle(SkPoint::Make(SafeNarrow(x), SafeNarrow(y)),
                           SafeNarrow(radius), dl_paint);
  }
}

void Canvas::drawArc(double left,
                     double top,
                     double right,
                     double bottom,
                     double startAngle,
                     double sweepAngle,
                     bool useCenter,
                     Dart_Handle paint_objects,
                     Dart_Handle paint_data) {
  Paint paint(paint_objects, paint_data);

  FML_DCHECK(paint.isNotNull());
  if (dl_canvas_) {
    DlPaint dl_paint;
    paint.paint(dl_paint, useCenter  //
                              ? kDrawArcWithCenterFlags
                              : kDrawArcNoCenterFlags);
    dl_canvas_->DrawArc(
        SkRect::MakeLTRB(SafeNarrow(left), SafeNarrow(top), SafeNarrow(right),
                         SafeNarrow(bottom)),
        SafeNarrow(startAngle) * 180.0f / static_cast<float>(M_PI),
        SafeNarrow(sweepAngle) * 180.0f / static_cast<float>(M_PI), useCenter,
        dl_paint);
  }
}

void Canvas::drawPath(const CanvasPath* path,
                      Dart_Handle paint_objects,
                      Dart_Handle paint_data) {
  Paint paint(paint_objects, paint_data);

  FML_DCHECK(paint.isNotNull());
  if (!path) {
    Dart_ThrowException(
        ToDart("Canvas.drawPath called with non-genuine Path."));
    return;
  }
  if (dl_canvas_) {
    DlPaint dl_paint;
    paint.paint(dl_paint, kDrawPathFlags);
    dl_canvas_->DrawPath(path->path(), dl_paint);
  }
}

Dart_Handle Canvas::drawImage(const CanvasImage* image,
                              double x,
                              double y,
                              Dart_Handle paint_objects,
                              Dart_Handle paint_data,
                              int filterQualityIndex) {
  Paint paint(paint_objects, paint_data);

  FML_DCHECK(paint.isNotNull());
  if (!image) {
    return ToDart("Canvas.drawImage called with non-genuine Image.");
  }

  auto dl_image = image->image();
  if (!dl_image) {
    return Dart_Null();
  }
  auto error = dl_image->get_error();
  if (error) {
    return ToDart(error.value());
  }

  auto sampling = ImageFilter::SamplingFromIndex(filterQualityIndex);
  if (dl_canvas_) {
    DlPaint dl_paint;
    const DlPaint* opt_paint = paint.paint(dl_paint, kDrawImageWithPaintFlags);
    dl_canvas_->DrawImage(dl_image, SkPoint::Make(SafeNarrow(x), SafeNarrow(y)),
                          sampling, opt_paint);
  }
  return Dart_Null();
}

Dart_Handle Canvas::drawImageRect(const CanvasImage* image,
                                  double src_left,
                                  double src_top,
                                  double src_right,
                                  double src_bottom,
                                  double dst_left,
                                  double dst_top,
                                  double dst_right,
                                  double dst_bottom,
                                  Dart_Handle paint_objects,
                                  Dart_Handle paint_data,
                                  int filterQualityIndex) {
  Paint paint(paint_objects, paint_data);

  FML_DCHECK(paint.isNotNull());
  if (!image) {
    return ToDart("Canvas.drawImageRect called with non-genuine Image.");
  }

  auto dl_image = image->image();
  if (!dl_image) {
    return Dart_Null();
  }
  auto error = dl_image->get_error();
  if (error) {
    return ToDart(error.value());
  }

  SkRect src = SkRect::MakeLTRB(SafeNarrow(src_left), SafeNarrow(src_top),
                                SafeNarrow(src_right), SafeNarrow(src_bottom));
  SkRect dst = SkRect::MakeLTRB(SafeNarrow(dst_left), SafeNarrow(dst_top),
                                SafeNarrow(dst_right), SafeNarrow(dst_bottom));
  auto sampling = ImageFilter::SamplingFromIndex(filterQualityIndex);
  if (dl_canvas_) {
    DlPaint dl_paint;
    const DlPaint* opt_paint =
        paint.paint(dl_paint, kDrawImageRectWithPaintFlags);
    dl_canvas_->DrawImageRect(dl_image, src, dst, sampling, opt_paint,
                              DlCanvas::SrcRectConstraint::kFast);
  }
  return Dart_Null();
}

Dart_Handle Canvas::drawImageNine(const CanvasImage* image,
                                  double center_left,
                                  double center_top,
                                  double center_right,
                                  double center_bottom,
                                  double dst_left,
                                  double dst_top,
                                  double dst_right,
                                  double dst_bottom,
                                  Dart_Handle paint_objects,
                                  Dart_Handle paint_data,
                                  int bitmapSamplingIndex) {
  Paint paint(paint_objects, paint_data);

  FML_DCHECK(paint.isNotNull());
  if (!image) {
    return ToDart("Canvas.drawImageNine called with non-genuine Image.");
  }
  auto dl_image = image->image();
  if (!dl_image) {
    return Dart_Null();
  }
  auto error = dl_image->get_error();
  if (error) {
    return ToDart(error.value());
  }

  SkRect center =
      SkRect::MakeLTRB(SafeNarrow(center_left), SafeNarrow(center_top),
                       SafeNarrow(center_right), SafeNarrow(center_bottom));
  SkIRect icenter;
  center.round(&icenter);
  SkRect dst = SkRect::MakeLTRB(SafeNarrow(dst_left), SafeNarrow(dst_top),
                                SafeNarrow(dst_right), SafeNarrow(dst_bottom));
  auto filter = ImageFilter::FilterModeFromIndex(bitmapSamplingIndex);
  if (dl_canvas_) {
    DlPaint dl_paint;
    const DlPaint* opt_paint =
        paint.paint(dl_paint, kDrawImageNineWithPaintFlags);
    dl_canvas_->DrawImageNine(dl_image, icenter, dst, filter, opt_paint);
  }
  return Dart_Null();
}

void Canvas::drawPicture(Picture* picture) {
  if (!picture) {
    Dart_ThrowException(
        ToDart("Canvas.drawPicture called with non-genuine Picture."));
    return;
  }

  if (!dl_canvas_) {
    return;
  }

  if (picture->display_list()) {
    dl_canvas_->DrawDisplayList(picture->display_list());
  } else {
    auto impeller_picture = picture->impeller_picture();
    if (impeller_picture) {
      dl_canvas_->DrawImpellerPicture(impeller_picture);
    } else {
      FML_DCHECK(false);
    }
  }
}

void Canvas::drawPoints(Dart_Handle paint_objects,
                        Dart_Handle paint_data,
                        DlCanvas::PointMode point_mode,
                        const tonic::Float32List& points) {
  Paint paint(paint_objects, paint_data);

  static_assert(sizeof(SkPoint) == sizeof(float) * 2,
                "SkPoint doesn't use floats.");

  FML_DCHECK(paint.isNotNull());
  if (dl_canvas_) {
    DlPaint dl_paint;
    switch (point_mode) {
      case DlCanvas::PointMode::kPoints:
        paint.paint(dl_paint, kDrawPointsAsPointsFlags);
        break;
      case DlCanvas::PointMode::kLines:
        paint.paint(dl_paint, kDrawPointsAsLinesFlags);
        break;
      case DlCanvas::PointMode::kPolygon:
        paint.paint(dl_paint, kDrawPointsAsPolygonFlags);
        break;
    }
    dl_canvas_->DrawPoints(point_mode,
                           points.num_elements() / 2,  // SkPoints have 2 floats
                           reinterpret_cast<const SkPoint*>(points.data()),
                           dl_paint);
  }
}

void Canvas::drawVertices(const Vertices* vertices,
                          DlBlendMode blend_mode,
                          Dart_Handle paint_objects,
                          Dart_Handle paint_data) {
  Paint paint(paint_objects, paint_data);

  if (!vertices) {
    Dart_ThrowException(
        ToDart("Canvas.drawVertices called with non-genuine Vertices."));
    return;
  }
  FML_DCHECK(paint.isNotNull());
  if (dl_canvas_) {
    DlPaint dl_paint;
    paint.paint(dl_paint, kDrawVerticesFlags);
    dl_canvas_->DrawVertices(vertices->vertices(), blend_mode, dl_paint);
  }
}

Dart_Handle Canvas::drawAtlas(Dart_Handle paint_objects,
                              Dart_Handle paint_data,
                              int filterQualityIndex,
                              CanvasImage* atlas,
                              Dart_Handle transforms_handle,
                              Dart_Handle rects_handle,
                              Dart_Handle colors_handle,
                              DlBlendMode blend_mode,
                              Dart_Handle cull_rect_handle) {
  Paint paint(paint_objects, paint_data);

  if (!atlas) {
    return ToDart(
        "Canvas.drawAtlas or Canvas.drawRawAtlas called with "
        "non-genuine Image.");
  }

  auto dl_image = atlas->image();
  auto error = dl_image->get_error();
  if (error) {
    return ToDart(error.value());
  }

  static_assert(sizeof(SkRSXform) == sizeof(float) * 4,
                "SkRSXform doesn't use floats.");
  static_assert(sizeof(SkRect) == sizeof(float) * 4,
                "SkRect doesn't use floats.");

  auto sampling = ImageFilter::SamplingFromIndex(filterQualityIndex);

  FML_DCHECK(paint.isNotNull());
  if (dl_canvas_) {
    tonic::Float32List transforms(transforms_handle);
    tonic::Float32List rects(rects_handle);
    tonic::Int32List colors(colors_handle);
    tonic::Float32List cull_rect(cull_rect_handle);

    DlPaint dl_paint;
    const DlPaint* opt_paint = paint.paint(dl_paint, kDrawAtlasWithPaintFlags);
    dl_canvas_->DrawAtlas(
        dl_image, reinterpret_cast<const SkRSXform*>(transforms.data()),
        reinterpret_cast<const SkRect*>(rects.data()),
        reinterpret_cast<const DlColor*>(colors.data()),
        rects.num_elements() / 4,  // SkRect have four floats.
        blend_mode, sampling, reinterpret_cast<const SkRect*>(cull_rect.data()),
        opt_paint);
  }
  return Dart_Null();
}

void Canvas::drawShadow(const CanvasPath* path,
                        SkColor color,
                        double elevation,
                        bool transparentOccluder) {
  if (!path) {
    Dart_ThrowException(
        ToDart("Canvas.drawShader called with non-genuine Path."));
    return;
  }

  // Not using SafeNarrow because DPR will always be a relatively small number.
  SkScalar dpr = static_cast<float>(UIDartState::Current()
                                        ->platform_configuration()
                                        ->get_window(0)
                                        ->viewport_metrics()
                                        .device_pixel_ratio);
  if (dl_canvas_) {
    // The DrawShadow mechanism results in non-public operations to be
    // performed on the canvas involving an SkDrawShadowRec. Since we
    // cannot include the header that defines that structure, we cannot
    // record an operation that it injects into an SkCanvas. To prevent
    // that situation we bypass the canvas interface and inject the
    // shadow parameters directly into the underlying DisplayList.
    // See: https://bugs.chromium.org/p/skia/issues/detail?id=12125
    dl_canvas_->DrawShadow(path->path(), color, SafeNarrow(elevation),
                           transparentOccluder, dpr);
  }
}

void Canvas::Invalidate() {
  dl_canvas_ = nullptr;
  if (dart_wrapper()) {
    ClearDartWrapper();
  }
}

}  // namespace flutter
