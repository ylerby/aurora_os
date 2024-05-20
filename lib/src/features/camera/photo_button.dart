import 'package:flutter/material.dart';

import '../../theme/constants/constants.dart';

class PhotoButton extends StatelessWidget {
  final VoidCallback? onPressed;
  const PhotoButton({
    required this.onPressed,
    Key? key,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) => Container(
        padding: const EdgeInsets.all(3),
        decoration: BoxDecoration(
          border: Border.all(
            color: TopGColors.yLightGrey,
            width: 3,
          ),
          shape: BoxShape.circle,
        ),
        child: MaterialButton(
          onPressed: onPressed,
          color: TopGColors.yLightGrey,
          shape: const CircleBorder(),
        ),
      );
}
