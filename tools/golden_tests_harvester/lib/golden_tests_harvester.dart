// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;

import 'package:path/path.dart' as p;
import 'package:skia_gold_client/skia_gold_client.dart';

import 'src/digests_json_format.dart';

/// Used by [harvest] to process a directory for Skia Gold upload.
abstract class Harvester {
  /// Creates a new [Harvester] from the directory at [workDirectory].
  ///
  /// The directory is expected to match the following structure:
  /// ```txt
  /// workDirectory/
  ///   - digest.json
  ///   - test_name_1.png
  ///   - test_name_2.png
  ///   - ...
  /// ```
  ///
  /// The format of `digest.json` is expected to match the following:
  /// ```jsonc
  /// {
  ///   "dimensions": {
  ///     // Key-value pairs of dimensions to provide to Skia Gold.
  ///     // For example:
  ///     "platform": "linux",
  ///   },
  ///   "entries": [
  ///     // Each entry is a test-run with the following format:
  ///     {
  ///       // Path must be a direct sibling of digest.json.
  ///       "filename": "test_name_1.png",
  ///
  ///       // Called `screenshotSize` in Skia Gold (width * height).
  ///       "width": 100,
  ///       "height": 100,
  ///
  ///       // Called `differentPixelsRate` in Skia Gold.
  ///       "maxDiffPixelsPercent": 0.01,
  ///
  ///       // Called `pixelColorDelta` in Skia Gold.
  ///       "maxColorDelta": 0
  ///     }
  ///   ]
  /// }
  /// ```
  static Future<Harvester> create(
      io.Directory workDirectory, StringSink stderr,
      {AddImageToSkiaGold? addImageToSkiaGold}) async {
    final io.File file = io.File(p.join(workDirectory.path, 'digest.json'));
    if (!file.existsSync()) {
      // Check if the directory exists or if the file is just missing.
      if (!workDirectory.existsSync()) {
        throw ArgumentError('Directory not found: ${workDirectory.path}.');
      }
      // Lookup sibling files to help the user understand what's missing.
      final List<io.FileSystemEntity> files = workDirectory.listSync();
      throw StateError(
        'File "digest.json" not found in ${workDirectory.path}.\n\n'
        'Found files: ${files.map((io.FileSystemEntity e) => p.basename(e.path)).join(', ')}',
      );
    }
    final Digests digests = Digests.parse(file.readAsStringSync());

    if (addImageToSkiaGold != null) {
      return VanillaHarvester(digests, stderr, workDirectory, addImageToSkiaGold);
    } else {
      return SkiaGoldHarvester.create(digests, stderr, workDirectory);
    }
  }

  Future<void> addImg(
    String testName,
    io.File goldenFile, {
    double differentPixelsRate,
    int pixelColorDelta,
    required int screenshotSize,
  });

  Digests get digests;
  StringSink get stderr;
  io.Directory get workDirectory;
}

/// A [Harvester] that communicates with a real [SkiaGoldHarvester].
class SkiaGoldHarvester implements Harvester {
  final Digests digests;
  final StringSink stderr;
  final io.Directory workDirectory;
  final SkiaGoldClient client;

  static Future<SkiaGoldHarvester> create(
      Digests digests, StringSink stderr, io.Directory workDirectory) async {
    // If GOLDCTL is not configured (i.e. on CI), this will throw.
    final SkiaGoldClient client =
        SkiaGoldClient(workDirectory, dimensions: digests.dimensions);
    await client.auth();
    return SkiaGoldHarvester._init(digests, stderr, workDirectory, client);
  }

  SkiaGoldHarvester._init(
      this.digests, this.stderr, this.workDirectory, this.client);

  @override
  Future<void> addImg(String testName, io.File goldenFile,
      {double differentPixelsRate = 0.01,
      int pixelColorDelta = 0,
      required int screenshotSize}) {
    return client.addImg(testName, goldenFile,
        differentPixelsRate: differentPixelsRate,
        pixelColorDelta: pixelColorDelta,
        screenshotSize: screenshotSize);
  }
}

/// A [Harvester] that doesn't harvest, just logs to [stderr].
class VanillaHarvester implements Harvester {
  final Digests digests;
  final StringSink stderr;
  final io.Directory workDirectory;
  final AddImageToSkiaGold addImageToSkiaGold;
  VanillaHarvester(
      this.digests, this.stderr, this.workDirectory, this.addImageToSkiaGold);

  @override
  Future<void> addImg(String testName, io.File goldenFile,
      {double differentPixelsRate = 0.01,
      int pixelColorDelta = 0,
      required int screenshotSize}) async {
    return addImageToSkiaGold(testName, goldenFile,
        differentPixelsRate: differentPixelsRate,
        pixelColorDelta: pixelColorDelta,
        screenshotSize: screenshotSize);
  }
}

/// Uploads the images of digests  in [workDirectory] to Skia Gold.
Future<void> harvest(Harvester harvester) async {
  final List<Future<void>> pendingComparisons = <Future<void>>[];
  for (final DigestEntry entry in harvester.digests.entries) {
    final io.File goldenFile =
        io.File(p.join(harvester.workDirectory.path, entry.filename));
    final Future<void> future = harvester
        .addImg(
      entry.filename,
      goldenFile,
      screenshotSize: entry.width * entry.height,
      differentPixelsRate: entry.maxDiffPixelsPercent,
      pixelColorDelta: entry.maxColorDelta,
    )
        .catchError((Object e) {
      harvester.stderr.writeln('Failed to add image to Skia Gold: $e');
      throw FailedComparisonException(entry.filename);
    });
    pendingComparisons.add(future);
  }

  await Future.wait(pendingComparisons);
}

/// An exception thrown when a comparison fails.
final class FailedComparisonException implements Exception {
  /// Creates a new instance of [FailedComparisonException].
  const FailedComparisonException(this.testName);

  /// The test name that failed.
  final String testName;

  @override
  String toString() => 'Failed comparison: $testName';
}

/// A function that uploads an image to Skia Gold.
typedef AddImageToSkiaGold = Future<void> Function(
  String testName,
  io.File goldenFile, {
  double differentPixelsRate,
  int pixelColorDelta,
  required int screenshotSize,
});
