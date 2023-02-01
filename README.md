# Vulkan_Template

-Les objets Vulkan sont soit créés directement avec des fonctions comme vkCreateXXX,
    soit alloués via un autre objet avec des fonctions comme vkAllocateXXX.
    Après vous être assuré qu'un objet n'est plus utilisé nulle part,
    vous devez le détruire avec les contreparties vkDestroyXXXet vkFreeXXX.

----------------------------------------------------------------------------------------

-La toute première chose que vous devez faire est d'initialiser la bibliothèque
    Vulkan en créant une instance . L'instance est la connexion entre votre application
    et la bibliothèque Vulkan et sa création implique de spécifier certains détails
    sur votre application au pilote.

-Vulkan introduit un système élégant pour cela connu sous le nom de couches de validation .
    Les couches de validation sont des composants facultatifs qui se connectent aux appels de
    fonction Vulkan pour appliquer des opérations supplémentaires. Les opérations courantes dans
    les couches de validation sont :

    *Vérification des valeurs des paramètres par rapport à la spécification pour détecter une mauvaise utilisation
    *Suivi de la création et de la destruction d'objets pour trouver des fuites de ressources
    *Vérification de la sécurité des threads en suivant les threads d'où proviennent les appels
    *Journalisation de chaque appel et de ses paramètres sur la sortie standard
    *Le traçage de Vulkan appelle au profilage et à la relecture

-Les commandes dans Vulkan, comme les opérations de dessin et les transferts de mémoire,
    ne sont pas exécutées directement à l'aide d'appels de fonction. Vous devez enregistrer
    toutes les opérations que vous souhaitez effectuer dans les objets du tampon de commandes.
    L'avantage de ceci est que lorsque nous sommes prêts à dire à Vulkan ce que nous voulons
    faire, toutes les commandes sont soumises ensemble et Vulkan peut traiter plus efficacement
    les commandes car elles sont toutes disponibles ensemble. De plus, cela permet à l'enregistrement
    des commandes de se produire dans plusieurs threads si vous le souhaitez.

-À un niveau élevé, le rendu d'un cadre dans Vulkan consiste en un ensemble commun d'étapes :

    *Attendez que l'image précédente se termine
    *Acquérir une image de la chaîne d'échange
    *Enregistrez un tampon de commande qui dessine la scène sur cette image
    *Soumettez le tampon de commande enregistré
    *Présenter l'image de la chaîne d'échange

-il y a un certain nombre d'événements que nous devons ordonner explicitement parce qu'ils
    se produisent sur le GPU, tels que :

    *Acquérir une image de la chaîne d'échange
    *Exécuter des commandes qui s'appuient sur l'image acquise
    *Présentez cette image à l'écran pour la présentation, en la renvoyant à la chaîne d'échange

----------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------
NOTE:
------------------------------------------------------

Plusieurs sets de descripteurs
Comme on a pu le voir dans les en-têtes de certaines fonctions, il est possible de lier plusieurs sets de descripteurs en même temps. Vous devez fournir une organisation pour chacun des sets pendant la mise en place de l'organisation de la pipeline. Les shaders peuvent alors accéder aux descripteurs de la manière suivante :

layout(set = 0, binding = 0) uniform UniformBufferObject { ... }
Vous pouvez utiliser cette possibilité pour placer dans différents sets les descripteurs dépendant d'objets et les descripteurs partagés. De cette manière vous éviter de relier constemment une partie des descripteurs, ce qui peut être plus performant.

----------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------
OPTIMISATION:
------------------------------------------------------

#1:
La commande de copie de tampon nécessite une famille de files d'attente qui prend en charge
les opérations de transfert, ce qui est indiqué à l'aide de VK_QUEUE_TRANSFER_BIT.
La bonne nouvelle est que toute famille de files d'attente avec VK_QUEUE_GRAPHICS_BIT ou
VK_QUEUE_COMPUTE_BIT capacités prend déjà implicitement en charge les VK_QUEUE_TRANSFER_BITopérations.
L'implémentation n'est pas obligée de l'énumérer explicitement queueFlagsdans ces cas.

Si vous aimez les défis, vous pouvez toujours essayer d'utiliser une autre famille de
files d'attente spécifiquement pour les opérations de transfert. Il vous demandera d'apporter
les modifications suivantes à votre programme :

-Modifiez QueueFamilyIndiceset findQueueFamiliesrecherchez explicitement une famille de
    files d'attente avec le VK_QUEUE_TRANSFER_BITbit, mais pas le VK_QUEUE_GRAPHICS_BIT.
-Modifier createLogicalDevicepour demander un handle vers la file d'attente de transfert
    Créer un deuxième pool de commandes pour les tampons de commandes qui sont soumis sur la
    famille de files d'attente de transfert
-Modifiez les sharingModeressources en VK_SHARING_MODE_CONCURRENTet spécifiez à la fois
    les graphiques et les familles de file d'attente de transfert
-Soumettez toutes les commandes de transfert comme vkCmdCopyBuffer(que nous utiliserons dans ce chapitre)
    à la file d'attente de transfert au lieu de la file d'attente graphique

C'est un peu de travail, mais cela vous en apprendra beaucoup sur la façon dont les ressources
    sont partagées entre les familles de file d'attente.

------------------------------------------------------

#2
Il convient de noter que dans une application du monde réel, vous n'êtes pas censé appeler
vkAllocateMemorychaque tampon individuel. Le nombre maximal d'allocations de mémoire simultanées
est limité par la maxMemoryAllocationCount limite de périphérique physique, qui peut être aussi
faible que 4096même sur du matériel haut de gamme comme une NVIDIA GTX 1080. La bonne façon d'allouer
de la mémoire pour un grand nombre d'objets en même temps est de créez un alternateur personnalisé
qui divise une allocation unique entre plusieurs objets différents en utilisant les offsetparamètres
que nous avons vus dans de nombreuses fonctions.

Vous pouvez soit implémenter vous-même un tel alternateur, soit utiliser la bibliothèque
VulkanMemoryAllocator fournie par l'initiative GPUOpen. Cependant, pour ce didacticiel, vous pouvez
utiliser une allocation distincte pour chaque ressource, car nous ne serons pas près d'atteindre
l'une de ces limites pour le moment.

------------------------------------------------------

#3

Le chapitre précédent a déjà mentionné que vous devriez allouer plusieurs ressources comme des tampons
à partir d'une seule allocation de mémoire, mais en fait, vous devriez aller plus loin. Les
développeurs de pilotes recommandent également de stocker plusieurs tampons, tels que le tampon de
vertex et d'index, dans un seul VkBufferet d'utiliser des décalages dans des commandes telles que
vkCmdBindVertexBuffers. L'avantage est que vos données sont plus conviviales pour le cache dans ce cas,
car elles sont plus proches les unes des autres. Il est même possible de réutiliser le même morceau de
mémoire pour plusieurs ressources si elles ne sont pas utilisées lors des mêmes opérations de rendu,
à condition bien sûr de rafraîchir leurs données. C'est ce qu'on appelle l' aliasing et certaines
fonctions Vulkan ont des indicateurs explicites pour spécifier que vous voulez faire cela.
