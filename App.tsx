/**
 * Sample React Native App
 * https://github.com/facebook/react-native
 *
 * Generated with the TypeScript template
 * https://github.com/react-native-community/react-native-template-typescript
 *
 * @format
 */

import {
  LocalSourceImage,
  SketchCanvas,
} from '@terrylinla/react-native-sketch-canvas';
import React from 'react';
import {
  Button,
  SafeAreaView,
  StyleSheet,
  ScrollView,
  View,
  Text,
  StatusBar,
  NativeSyntheticEvent,
  NativeTouchEvent,
  NativeModules,
} from 'react-native';

import {
  Header,
  LearnMoreLinks,
  Colors,
  DebugInstructions,
  ReloadInstructions,
} from 'react-native/Libraries/NewAppScreen';

const App = () => {
  const [hasPickedFile, setHasPickedFile] = React.useState<boolean>(false);
  const [pickedFilePath, setPickedFilePath] = React.useState<string>('');
  const [sketchRef, setSketchRef] =
    React.useState<React.LegacyRef<SketchCanvas>>(null);
  const [localSourceImage, setLocalSourceImage] =
    React.useState<LocalSourceImage | undefined>(undefined);

  const onPressChooseImage = (ev?: NativeSyntheticEvent<NativeTouchEvent>) => {
    NativeModules.FilePicker.pickFile()
      .then((result: string) => {
        setHasPickedFile(true);
        // you can try without resizeImage by just removing this and passing the pickFile result as filename to localSourceImage
        // resizeImage(path, newMaxSide, quality, rotation, onlyScaleDown)
        NativeModules.ImageResizer.resizeImage(result, 1920, 80, 0, true)
          .then((resizedImagePath: string) => {
            setPickedFilePath(resizedImagePath);
            setLocalSourceImage({
              filename: resizedImagePath,
              directory: null,
              mode: 'AspectFit',
            });
          })
          .catch((error: any) => {
            console.log(error);
            onPressClearImage(undefined);
          });
      })
      .catch((error: any) => {
        console.log(error);
        onPressClearImage(undefined);
      });
  };

  const onPressClearImage = (ev?: NativeSyntheticEvent<NativeTouchEvent>) => {
    setHasPickedFile(false);
    setPickedFilePath('');
    setLocalSourceImage(undefined);
  };

  const onPressSaveImage = (ev?: NativeSyntheticEvent<NativeTouchEvent>) => {
    if (sketchRef && sketchRef !== null && sketchRef instanceof SketchCanvas) {
      console.log(sketchRef);
      const savePath = 'rnSketchTestTemp';
      sketchRef?.save(
        'jpg',
        false,
        savePath,
        String(Math.ceil(Math.random() * 100000000)),
        true,
        false,
        true,
      );
    }
  };

  const onImageSaved = (result: boolean, path: string) => {
    console.log(`saveSuccess: ${result}`);
  };

  const onRefReady = (ref: SketchCanvas) => {
    setSketchRef(ref);
  };

  return (
    <>
      <StatusBar barStyle="dark-content" />
      <SafeAreaView>
        <View
          style={{
            height: '100%',
            justifyContent: 'space-between',
          }}>
          {hasPickedFile ? (
            <Button title={'Save image'} onPress={onPressSaveImage} />
          ) : (
            <Button title={'Pick image'} onPress={onPressChooseImage} />
          )}
          {localSourceImage && pickedFilePath && pickedFilePath.length > 0 ? (
            <SketchCanvas
              ref={onRefReady}
              style={{flex: 1}}
              localSourceImage={localSourceImage}
              onSketchSaved={onImageSaved}
            />
          ) : (
            React.Fragment
          )}
          {hasPickedFile ? (
            <Button title={'Reset'} onPress={onPressClearImage} />
          ) : (
            React.Fragment
          )}
        </View>
      </SafeAreaView>
    </>
  );
};

const styles = StyleSheet.create({
  scrollView: {
    backgroundColor: Colors.lighter,
  },
  engine: {
    position: 'absolute',
    right: 0,
  },
  body: {
    backgroundColor: Colors.white,
  },
  sectionContainer: {
    marginTop: 32,
    paddingHorizontal: 24,
  },
  sectionTitle: {
    fontSize: 24,
    fontWeight: '600',
    color: Colors.black,
  },
  sectionDescription: {
    marginTop: 8,
    fontSize: 18,
    fontWeight: '400',
    color: Colors.dark,
  },
  highlight: {
    fontWeight: '700',
  },
  footer: {
    color: Colors.dark,
    fontSize: 12,
    fontWeight: '600',
    padding: 4,
    paddingRight: 12,
    textAlign: 'right',
  },
});

export default App;
