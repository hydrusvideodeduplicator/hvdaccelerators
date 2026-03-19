# Development

## Setup

[Hatch](https://hatch.pypa.io/latest/) is the Python project manager for this project.

It is used for packaging and environment management for development.

There are hatch commands for this project are defined in the [pyproject.toml]. They are just aliases for other commands that run in specific environments.

For example, to run the command to run all tests:

```sh
hatch run test:all
```

`test` is the environment and `all` is the command to run.

For more information, see the Hatch [environment documentation](https://hatch.pypa.io/latest/environment/)

### Useful commands

Run all tests:

```sh
hatch run test:all
```

Check code formatting (doesn't actually run formatting):

```sh
hatch run lint:format
```

Lint code:

```sh
hatch run lint:lint
```

Format code:

```sh
hatch run format:format
```

### Testing

## hvdaccelerators tests

Run tests with `hatch run test:all`

## Integration Tests with Hydrus Video Deduplicator

To test hvdaccelerators changes in Hydrus Video Deduplicator using GitHub Actions, make a branch off
the develop branch in the hydrus-video-deduplicator repo and set the dependency version of hvdaccelerators
to the git url for the branch to test, and set hatch to allow direct references.

`hydrus-video-deduplicator/pyproject.toml`:

```text
...
dependencies = [
    "hvdaccelerators@git+https://github.com/hydrusvideodeduplicator/hvdaccelerators.git@branch-to-test",
]

[tool.hatch.metadata]
allow-direct-references = true
```

Then push this branch and GitHub Actions will use that branch for hydrus-video-deduplicator tests.

## git workflow

Create PRs for `develop` if you want to submit changes.

## Maintainer Release Process

This release process section is for the maintainer.

Use the `develop` branch for development. Do not rewrite history on `develop` because others may be branched from it.
During development, if you think you might need to rewrite history, then create a branch off of `develop` and use that.
Commits to `develop` should be "clean"; do not add "WIP" commits to develop.

Once `develop` has all the changes you want to publish, follow the steps below.

Following semantic versioning rules, increment set the new version in `src/hvdaccelerators/__about__.py` and in `pyproject.toml`.

Add a commit with only the new version:

```sh
git add src/hvdaccelerators/__about__.py pyproject.toml
git commit -m "Release X.Y.Z"
```

Add a git tag to the new commit:

```sh
git tag -a "vX.Y.Z" -m "vX.Y.Z"
```

Push the changes from develop and the tag:

```sh
git push
git push origin tag "vX.Y.Z"
```

Once the GitHub Actions are done running and everything passes, a release needs to be made on the GitHub Release page.
Follow the formatting from previous releases. A GitHub release will trigger a PyPI publish for the new package build.
